#include <iostream>
#include <memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <curl/curl.h>

#include "kafkaclient.h"
#include "restclient/restclient.h"
#include "rabbitmqclient.h"
#include "taskparams.h"
#include "taskresult.h"
#include "util.h"
#include "ossclient.h"
#include "googleclient.h"
#include "config.h"
#include "staticservice.h"
#include "logger.h"
#include "pictool.h"
#include "common.h"
#include "pushservice.h"
#include "include/MetricsEx.h"
#include "Bs2Client.h"

// 算法库
#include "ailib/cover_qc_models_api.h"

using namespace cv;
using namespace yyai;

std::string g_model_dir;
std::string g_tmp_dir = "/dev/shm/tmp_videos";
int g_device_num = 0;
int g_save_on_fail = 0;
int g_save_all = 0;
int g_worker_threads_num = 1;
int g_pool_size = 1;
int g_video_fps = 5;

Config g_config;
const std::string BS2_UPLOAD_HOST = "bs2ul.yy.com";
const std::string BS2_DOWNLOAD_HOST = "bs2dl.yy.com";

yyai::CoverQualityModelsApi *g_attr_model = NULL;

//算法前处理函数 begin-----------------------------------------------------------
bool get_video_scale(int &target_width, int &target_height) {
    const int min_size = 360;

    if (target_width < target_height) {
        int tmp = target_width;
        target_width = min_size;
        target_height = (int) (target_height * min_size / (float) tmp);
    } else {
        int tmp = target_height;
        target_height = min_size;
        target_width = (int) (target_width * min_size / (float) tmp);
    }

    return true;
}

bool
read_video_to_list2(const std::string &video_path, std::vector <cv::Mat> &frames, float &fps, int sample_interval = 1) {
    cv::VideoCapture video(video_path.c_str());
    if (!video.isOpened()) {
        return false;
    }
    const int min_size = 360;

    int target_width = int(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int target_height = int(video.get(cv::CAP_PROP_FRAME_HEIGHT));
    fps = video.get(cv::CAP_PROP_FPS);
    fps = fps > 0 ? fps : 1.0;
    int frame_count = int(video.get(cv::CAP_PROP_FRAME_COUNT));

    if (target_width < target_height) {
        int tmp = target_width;
        target_width = min_size;
        target_height = (int) (target_height * min_size / (float) tmp);
    } else {
        int tmp = target_height;
        target_height = min_size;
        target_width = (int) (target_width * min_size / (float) tmp);
    }

    // frames.resize(frame_count);
    for (int i = 0; i < frame_count; i++) {
        cv::Mat image;
        video >> image;
        if (image.empty()) {
            continue;
        }
        if (i % 10 != 0) {      // 每隔10帧取一帧
            continue;
        }
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(target_width, target_height));

        frames.push_back(resized);
    }
    fps = fps / 10;
    video.release();
    //printf("all_frames %d\n", frames.size());

    return true;
}

bool get_audio_wav(const std::string &video_path, const std::string &audio_save_path) {
    char cmd[2048] = {0};
    snprintf(cmd, sizeof(cmd),
             "./ffmpeg  -c:v h264_cuvid  -hwaccel_device  0  -threads 1 -hwaccel_device 0  -y -v 0 -t 120 -i %s %s",
             video_path.c_str(), audio_save_path.c_str());
    int exitCode = system(cmd);
    if (exitCode != 0) {
        return false;
    }

    return true;
}

bool read_directory(const std::string &name, std::vector <std::string> &v) {
    DIR *dirp = opendir(name.c_str());
    if (dirp == NULL) {
        return false;
    }

    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strncmp(dp->d_name, ".", 1) == 0 || strncmp(dp->d_name, "..", 1) == 0) {
            continue;
        }
        v.push_back(name + "/" + dp->d_name);
    }
    closedir(dirp);
    return true;
}

bool cmp(const std::string &x, const std::string &y) {
    return x < y;
}

bool get_frame_list(const char *traceid, const std::string &video_path, const std::string &frames_dir,
                    std::vector <std::string> &frames_file, int &width, int &height) {
    if (!get_video_scale(width, height)) {
        FUNLOG(Err, "%s get video scale failed", traceid);
        return false;
    }

    char cmd[2048] = {0};
    snprintf(cmd, sizeof(cmd),
             "mkdir -p %s && ./ffmpeg  -c:v h264_cuvid  -hwaccel_device  0 -y -v 0 -i %s -vsync 2 -f image2 -r %d -t 120 -vf scale=%d:%d -threads 1 %s/frames-%%04d.png",
             frames_dir.c_str(), video_path.c_str(), g_video_fps, width, height, frames_dir.c_str());
    int exitCode = system(cmd);
    if (exitCode != 0) {
        return false;
    }

    if (!read_directory(frames_dir, frames_file)) {
        return false;
    }
    if (frames_file.size() == 0) {
        return false;
    }
    std::sort(frames_file.begin(), frames_file.end(), cmp);

    return true;
}


//算法前处理函数 end-----------------------------------------------------------
long getCurrentTimeMicro() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
}

int save_meta_to_file(const char *data, int datalen, const char *filepath) {
    FILE *fp = fopen(filepath, "wb+");
    if (fp == NULL) {
        FUNLOG(Err, "can not open meta file: %s", filepath);
        return -1;
    }

    int write_size = fwrite(data, 1, datalen, fp);
    fclose(fp);
    FUNLOG(Info, "write data.len=%d, to file %s, has write %d", datalen, filepath, write_size);
    return write_size;
}

int process_one_file(const std::string &secretid, const char *traceid, InputConfig *input, OutputConfig *output,
                     const char *tmp_dir,
                     std::string &err_msg, std::string &out_file_url, std::string &results_json) {
    long downStartTime = 0, downEndTime = 0, downCostTime = 0;
    long predictStartTime = 0, predictEndTime = 0, predictCostTime = 0;
    long getFrameListStart = 0, getFrameListEnd = 0;
    long getWavStart = 0, getWavEnd = 0;
    long uploadStartTime = 0, uploadEndTime = 0, uploadCostTime = 0;
    char tmp_save_dir[256] = {0};
    sprintf(tmp_save_dir, "%s/%s", tmp_dir, getUuid().c_str());

    int create_fail = mkdir(tmp_save_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    if (create_fail) {
        FUNLOG(Err, "%s create dir fail! dir=%s", traceid, tmp_save_dir);
        err_msg = "create tmp dir fail!";
        return PROCESS_ERROR_CREATE_DIR;
    }

    FUNLOG(Info, "%s create tmp dir success! dir=%s", traceid, tmp_save_dir);

    int ret_value = PROCESS_ERROR_OK;
    try {
        do {
            std::vector <std::string> users_path;
            std::string local_file_path =
                    tmp_save_dir + std::string("/") + std::string(getUuid().c_str()) + std::string(".jpg");
            std::string out_file_path =
                    tmp_save_dir + std::string("/") + std::string(getUuid().c_str()) + std::string(".jpg");

            FUNLOG(Info, "%s, local_file_path=%s,  out_file_path=%s", traceid, local_file_path.c_str(),
                   out_file_path.c_str());

            int ret = PROCESS_ERROR_OK;
            bool pre_process_success = true;

            // download picture 
            FUNLOG(Info, "%s try to donwload url=%s", traceid, input->url.c_str());
            downStartTime = getCurrentTimeMicro();
            {
                MetricsAutoReporter report("i", &ret, (secretid + DOWNLOAD_FIEL).c_str());
                for (int rt = 0; rt < 3; rt++) {
                    ret = http_download(traceid, input->url.c_str(), local_file_path.c_str(),
                                        g_config.m_download_proxy_http.c_str(),
                                        g_config.m_download_proxy_https.c_str());
                    if (0 != ret) {
                        FUNLOG(Err, "%s download file fail!", traceid);
                        usleep(500000);
                        continue;
                    }
                    break;
                }

                if (0 != ret) {
                    ret_value = PROCESS_ERROR_DOWNLOAD;
                    err_msg = "download file fail!";
                    break;
                }
            }

            // check if exist file
            if (!isFileExist(local_file_path.c_str())) {
                ret_value = PROCESS_ERROR_DOWNLOAD_CHECK;
                err_msg = "download file not exist";
                FUNLOG(Err, "%s %s", traceid, err_msg.c_str());
                break;
            }

            // check if  file size is 0
            if (getFileSize(local_file_path.c_str()) <= 0) {
                ret_value = PROCESS_ERROR_DOWNLOAD_CHECK;
                err_msg = "download file size is 0";
                FUNLOG(Err, "%s %s", traceid, err_msg.c_str());
                break;
            }

            FUNLOG(Info, "%s download user file success! save to %s", traceid, local_file_path.c_str());
            downEndTime = getCurrentTimeMicro();

            if (PROCESS_ERROR_OK != ret_value) {
                FUNLOG(Err, "%s download part has fail!", traceid);
                break;
            }

            std::string dpi = "0*0";
            int video_width = -1;
            int video_height = -1;

            // predict
            bool success = true;
            predictStartTime = getCurrentTimeMicro();
            {
                MetricsAutoReporter report("i", &ret, (secretid + ALGORITHM_PRDICT).c_str());

                try {
                    FUNLOG(Info, "%s try to predict...", traceid);
                    int predict_ret = g_attr_model->inference("11111", local_file_path, results_json);

                    if (predict_ret != 0) {
                        success = false;
                    }

                    FUNLOG(Info, "%s inference predict_retreturn code: %d, return json:", traceid, predict_ret,
                           results_json.c_str());
                    FUNLOG(Info, "%s try to predict...finish", traceid);
                }
                catch (std::exception &e) {
                    err_msg = std::string("exctption! ") + std::string(e.what());
                    ret_value = PROCESS_ERROR_PREDICT;
                    FUNLOG(Err, "%s predict exception=%s", traceid, err_msg.c_str());
                    success = false;
                }
                catch (...) {
                    err_msg = "predict err";
                    ret_value = PROCESS_ERROR_PREDICT;
                    FUNLOG(Err, "%s predict exception", traceid);
                    success = false;
                }
            }

            if (!success) {
                ret_value = PROCESS_ERROR_PREDICT;
                err_msg = "algorithm predict fail!";
                FUNLOG(Err, "%s err_msg=%s", traceid, err_msg.c_str());
                break;
            }

            // // check if exist file
            // if(!isFileExist(out_file_path.c_str()))
            // {
            //     ret_value = PROCESS_ERROR_NO_VIDEO_OUTPUT;
            //     err_msg = "algorithm predict has no file output";
            //     FUNLOG(Err, "%s %s", traceid, err_msg.c_str());
            //     break;
            // }

            // // check if output video file size is 0
            // if(getFileSize(out_file_path.c_str()) <= 0)
            // {
            //     ret_value = PROCESS_ERROR_NO_VIDEO_OUTPUT;
            //     err_msg = "algorithm predict output file size is 0! please check GPU driver is higer than 318.";
            //     FUNLOG(Err, "%s %s", traceid, err_msg.c_str());
            //     break;
            // }

            predictEndTime = getCurrentTimeMicro();


            // // try to upload
            uploadStartTime = getCurrentTimeMicro();
            // std::string dst_file_name;
            // if(!input->dstfilename.empty())
            // {
            //     dst_file_name = input->dstfilename;
            // }
            // else
            // {
            //     dst_file_name = getUuid();
            // }
            // if(output->upload_type == UPLOAD_BS2)
            // {
            //     MetricsAutoReporter report("i", &ret, (secretid + UPLOAD_FILE_BS2).c_str());

            //     FUNLOG(Info, "%s try to upload file to bs2...", traceid);
            //     ret = PROCESS_ERROR_OK;
            //     CBs2Client client(traceid, BS2_UPLOAD_HOST, BS2_DOWNLOAD_HOST, output->bucket, output->secret_key, output->secret_code);

            //     std::ifstream in(out_file_path);
            //     std::ostringstream tmp;
            //     tmp << in.rdbuf();
            //     std::string data_str = tmp.str();
            //     FUNLOG(Info, "%s data size is %d", traceid, data_str.size());

            //     for(int rt=0; rt<3; rt++)
            //     {
            //         bool up_success = client.upload(data_str.c_str(),data_str.size(), dst_file_name, out_file_url);
            //         if(!up_success)
            //         {
            //             ret = PROCESS_ERROR_UPLOADFILE;
            //             FUNLOG(Err, "%s upload file fail! local file=%s", traceid, out_file_path.c_str());
            //             usleep(500000);
            //             continue;
            //         }
            //         FUNLOG(Info, "%s upload video file success! url=%s", traceid, out_file_url.c_str());
            //         break;
            //     }
            //     in.close();
            // }
            // else if(output->upload_type == UPLOAD_OSS)
            // {
            //     MetricsAutoReporter report("i", &ret, (secretid + UPLOAD_FILE_OSS).c_str());

            // 	FUNLOG(Info, "%s try to upload file to oss...", traceid);
            // 	ossclient client(output->secret_key, output->secret_code, output->end_point);
            //     for(int rt=0; rt<3; rt++)
            //     {
            //         ret = client.upload(out_file_path, dst_file_name, output->bucket, out_file_url, err_msg);
            //         if(0 != ret)
            //         {
            //             FUNLOG(Err, "%s upload file fail! local file=%s", traceid, out_file_path.c_str());
            //             usleep(500000);
            //             continue;
            //         }

            //         if(output->cdn_host != "")
            //             out_file_url = output->cdn_host + "/" + dst_file_name;

            //         FUNLOG(Info, "%s upload video file success! url=%s", traceid, out_file_url.c_str());
            //         break;
            //     }

            //     if(ret != PROCESS_ERROR_OK)
            //     {
            //         ret_value = PROCESS_ERROR_UPLOADFILE;
            //         err_msg = "upload file fail!";
            //         break;
            //     }
            // }
            // else if(output->upload_type == UPLOAD_GOOGLE)
            // {
            //     MetricsAutoReporter report("i", &ret, (secretid + UPLOAD_FILE_GOOGLE).c_str());

            //     // upload pciture file
            //     char* content_type = "image/jpg";
            //     FUNLOG(Info, "%s try to upload video file to google...", traceid);
            //     GoogleClient client;
            //     for(int rt=0; rt<3; rt++)
            //     {
            //         ret = client.upload(output->bucket.c_str(), out_file_path.c_str(), dst_file_name.c_str(), content_type);
            //         if(0 != ret)
            //         {
            //             FUNLOG(Err, "%s upload screenshot file fail! local file=%s", traceid, out_file_path.c_str());
            //             usleep(500000);
            //             continue;
            //         }

            //         out_file_url = output->cdn_host + "/" + dst_file_name;
            //         FUNLOG(Info, "%s upload screenshot file success! url=%s", traceid, out_file_url.c_str());
            //         break;
            //     }

            //     if(ret != PROCESS_ERROR_OK)
            //     {
            //         ret_value = PROCESS_ERROR_UPLOADFILE;
            //         err_msg = "upload file fail!";
            //         break;
            //     }
            // }
            // else
            // {
            // 	ret_value = PROCESS_ERROR_EXCEPTION;
            // 	err_msg = "not support this upload type now....";
            //     FUNLOG(Err, "%s not support this upload type now!", traceid);
            // 	break;
            // }

            uploadEndTime = getCurrentTimeMicro();
            //ret_value = PROCESS_ERROR_OK;
            FUNLOG(Info,
                   "%s url:%s, cost time, downCostTime: %ld, predictCostTime:%ld, getFrameListCostTime:%ld, getWavCostTime:%ld,uploadCostTime:%ld",
                   traceid, input->url.c_str(), (downEndTime - downStartTime) / 1000,
                   (predictEndTime - predictStartTime) / 1000, (getFrameListEnd - getFrameListStart) / 1000,
                   (getWavEnd - getWavStart) / 1000, (uploadEndTime - uploadStartTime) / 1000);
        } while (false);
    }
    catch (std::exception &e) {
        err_msg = std::string("exctption! ") + std::string(e.what());
        ret_value = PROCESS_ERROR_EXCEPTION;
        FUNLOG(Err, "%s exception=%s", traceid, err_msg.c_str());
    }

    // delete tmp dir
    if (0 == g_save_all && (g_save_on_fail == 0 || ret_value == PROCESS_ERROR_OK)) {
        FUNLOG(Info, "%s try remove tmp dir...", traceid);
        if (0 != remove_dir(tmp_save_dir)) {
            FUNLOG(Err, "%s rmdir fail! dir=%s", traceid, tmp_save_dir);
        }
    }

    return ret_value;
}

void callback_func(std::string &msg) {
    int ret = PROCESS_ERROR_OK;
    try {
        MetricsAutoReporter report("s", &ret, ALL_BUSINESS_PROCESS_REQUEST.c_str());
        long startTime = getCurrentTimeMicro();
        FUNLOG(Info, "callback_func in, data=%s", msg.c_str());

        TaskParams params;
        if (false == params.parse(msg)) {
            ret = PROCESS_ERROR_TASK_PARAMS;
            usleep(200000);
            FUNLOG(Err, "parse json fail! content=%s", msg.c_str());
            return;
        }

        MetricsAutoReporter report_with_secretid("s", &ret, (params.m_secretid + PROCESS_REQUEST).c_str());
        MetricsAutoReporter report3("i", &params.m_priority, (params.m_secretid + PRIORITY).c_str());
        // check taskid if match with config file
        //	if(params.m_taskid != g_config.m_taskid)
        //	{
        //		FUNLOG(Err, "[traceid:%s] taskid=%s not match config taskid=%s", params.m_traceid.c_str(), params.m_taskid.c_str(), g_config.m_taskid.c_str());
        //		return;
        //	}

        //StaticService::getInstance()->addMsgProcess(params.m_traceid, params.m_taskid, params.m_jobid);

        FinalResult final_result;
        final_result.traceid = params.m_traceid;
        final_result.taskid = params.m_taskid;

        for (int i = 0; i < params.m_inputs.size(); i++) {
            if (i > MAX_INPUT_SIZE) {
                MetricsAutoReporter report("i", &ret, (params.m_secretid + DATALIST_TOO_LARGE).c_str());
                FUNLOG(Info, "%s the index of dalalist is large than %d, will skip the left", params.m_traceid.c_str(),
                       MAX_INPUT_SIZE);
                break;
            }

            OneFileResult one_result;
            std::string err_msg = "ok";
            std::string out_url = "";
            std::string results_json = "";

            ret = PROCESS_ERROR_OK;
            {
                MetricsAutoReporter report1("i", &ret, (params.m_secretid + PROCESS_ONE_FILE).c_str());
                MetricsAutoReporter report2("i", &ret, ALL_BUSINESS_PROCESS_ONE_FIEL.c_str());

                ret = process_one_file(params.m_secretid, params.m_traceid.c_str(), &(params.m_inputs[i]),
                                       &(params.m_output), g_tmp_dir.c_str(), err_msg, out_url, results_json);
            }

            one_result.url = params.m_inputs[i].url;
            one_result.context = params.m_inputs[i].context;
            one_result.code = ret;
            one_result.msg = err_msg;
            one_result.result.out_url = out_url;
            one_result.result.predict_code = ret;

            Json::Reader t_reader;
            Json::Value t_root;
            if (ret == 0 && !t_reader.parse(results_json.c_str(), t_root)) {
                one_result.code = PROCESS_ERROR_PREDICT;
                one_result.msg = "result of algorith format is not json";
                FUNLOG(Err, "%s result of algorith format is not json, result %s", params.m_traceid.c_str(),
                       results_json.c_str());
            }
            one_result.result.predict_result = t_root;

            final_result.resultlist.push_back(one_result);

            if (0 != ret) {
                FUNLOG(Err, "%s process fail! err=%d, msg=%s", params.m_traceid.c_str(), ret, err_msg.c_str());
            } else {
                FUNLOG(Info, "%s process success!", params.m_traceid.c_str());
            }
        }

        std::string result_json = final_result.getJsonStr();
        FUNLOG(Info, "%s the result=%s", params.m_traceid.c_str(), result_json.c_str());

        // feedback publish
        if (g_config.m_kafka_hostport != "" && g_config.m_kafka_topic != "") {
            MetricsAutoReporter report("i", &ret, (params.m_secretid + FEEDBACK_PUBLISH).c_str());
            FUNLOG(Info, "%s try to feedback result to kafka server, hostport:%s, topic:%s",
                   params.m_traceid.c_str(), g_config.m_kafka_hostport.c_str(), g_config.m_kafka_topic.c_str());

            CKafkaClient client(params.m_traceid, g_config.m_kafka_hostport, g_config.m_kafka_topic,
                                result_json.c_str(), 3);
            bool success = client.sendMessage();
            if (success) {
                ret = PROCESS_ERROR_OK;
                FUNLOG(Info, "%s kafka feedback  push success!", params.m_traceid.c_str());
            } else {
                ret = PROCESS_ERROR_PUSH_RESULT_FAILED;
                FUNLOG(Err, "%s kafaka feedback push fail!", params.m_traceid.c_str());
            }
        }

        // check how to push result
        ret = PROCESS_ERROR_OK;
        if (params.m_output.push_type == PUSH_HTTP) {
            MetricsAutoReporter report("i", &ret, (params.m_secretid + PUBLISH_RESULT_HTTP).c_str());
            FUNLOG(Info, "%s try to push result to http server: %s", params.m_traceid.c_str(),
                   params.m_output.http_url.c_str());
            for (int x = 0; x < 2; x++) {
                RestClient::Response resp = RestClient::post(params.m_output.http_url.c_str(), "application/json",
                                                             result_json.c_str());
                if (200 != resp.code) {
                    ret = PROCESS_ERROR_PUSH_RESULT_FAILED;
                    FUNLOG(Err, "%s http push fail! url=%s, code=%d", params.m_traceid.c_str(),
                           params.m_output.http_url.c_str(), resp.code);
                    usleep(500000);
                    continue;
                }

                ret = PROCESS_ERROR_OK;
                FUNLOG(Info, "%s http push success!", params.m_traceid.c_str());
                break;
            }

            // if fail! add to retry buffer
            if (PROCESS_ERROR_OK != ret) {
                if (!PushService::getInstance()->addMsgToBuffer(params.m_traceid, params.m_output.http_url,
                                                                result_json)) {
                    FUNLOG(Err, "%s add to http push buffer fail!", params.m_traceid.c_str());
                } else {
                    FUNLOG(Info, "%s add to http push buffer success!", params.m_traceid.c_str());
                }
            }
        } else if (params.m_output.push_type == PUSH_KAFKA) {
            MetricsAutoReporter report("i", &ret, (params.m_secretid + PUBLISH_RESULT_KAFKA).c_str());
            FUNLOG(Info, "%s try to push result to kafka server, hostport:%s, topic:%s",
                   params.m_traceid.c_str(), params.m_output.kafka_hostport.c_str(),
                   params.m_output.kafka_topic.c_str());

            CKafkaClient client(params.m_traceid, params.m_output.kafka_hostport, params.m_output.kafka_topic,
                                result_json.c_str(), 3);
            bool success = client.sendMessage();
            if (success) {
                ret = PROCESS_ERROR_OK;
                FUNLOG(Info, "%s kafka push success!", params.m_traceid.c_str());
            } else {
                ret = PROCESS_ERROR_PUSH_RESULT_FAILED;
                FUNLOG(Err, "%s kafaka push fail!", params.m_traceid.c_str());
            }
        } else {
            FUNLOG(Err, "%s unsupport push type", params.m_traceid.c_str());
        }

        int costMs = (getCurrentTimeMicro() - startTime) / 1000;
        //StaticService::getInstance()->addMsgComplete(params.m_traceid, params.m_taskid, params.m_jobid, result_json, costMs);
    }
    catch (std::exception &e) {
        std::string msg = std::string("exctption! ") + std::string(e.what());
        FUNLOG(Err, "exception=%s", msg.c_str());
    }
}

int main(int argc, const char *argv[]) {
    init_log();

    g_device_num = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "-model") == 0) {
            g_model_dir = argv[i + 1];
            printf("model dir: %s\n", g_model_dir.c_str());
            FUNLOG(Info, "model dir: %s", g_model_dir.c_str());
        } else if (strcmp(argv[i], "-t") == 0) {
            g_tmp_dir = argv[i + 1];
            printf("tmp dir: %s\n", g_tmp_dir.c_str());
            FUNLOG(Info, "tmp dir: %s", g_tmp_dir.c_str());
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-device") == 0) {
            g_device_num = atoi(argv[i + 1]);
            if (g_device_num < 0)
                g_device_num = 0;
            printf("use device: %d\n", g_device_num);
            FUNLOG(Info, "use device: %d", g_device_num);
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-save") == 0) {
            g_save_on_fail = atoi(argv[i + 1]);
            printf("save on fail: %d\n", g_save_on_fail);
            FUNLOG(Info, "save on fail: %d", g_save_on_fail);
        } else if (strcmp(argv[i], "-r") == 0) {
            g_save_all = atoi(argv[i + 1]);
            printf("save all: %d\n", g_save_all);
            FUNLOG(Info, "save all: %d", g_save_all);
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "-work_num") == 0) {
            g_worker_threads_num = atoi(argv[i + 1]);
            printf("g_worker_threads_num: %d\n", g_worker_threads_num);
            FUNLOG(Info, "g_worker_threads_num: %d", g_worker_threads_num);
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-pool_size") == 0) {
            g_pool_size = atoi(argv[i + 1]);
            printf("g_pool_size: %d\n", g_pool_size);
            FUNLOG(Info, "g_pool_size: %d", g_pool_size);
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-fps") == 0) {
            g_video_fps = atoi(argv[i + 1]);
            printf("g_video_fps: %d\n", g_video_fps);
            FUNLOG(Info, "g_video_fps: %d", g_video_fps);
        } else if (strcmp(argv[i], "-h") == 0) {
            printf(" -m|assign model dir path"
                   "\n -d|bind gpu index"
                   "\n -s|save local file on fail"
                   "\n -h|help for help doc\n");
            return 0;
        }
    }

    //Must initialize libcurl before any threads are starte
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != res) {
        FUNLOG(Err, "init libcurl failed.");
        curl_global_cleanup();
        return -1;
    }

    //check if tmp dir is exist
    printf("check if tmp dir is exist\n");
    if (!isFileExist(g_tmp_dir.c_str())) {
        int create_fail = mkdir(g_tmp_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        if (create_fail) {
            FUNLOG(Err, "create tmp  dir fail! dir=%s", g_tmp_dir);
            return PROCESS_ERROR_CREATE_DIR;
        }
    }

    if (!g_config.load("../conf/config.json")) {
        printf("load config fail!\n");
        FUNLOG(Err, "load config fail!");
        return 1;
    }

    if (!MetricsBuffer::getInstance()->init(g_config.m_metrics_service_name.c_str()))
        FUNLOG(Err, "init metrics fail!");

    FUNLOG(Info, "config: metrics->servicename=%s", g_config.m_metrics_service_name.c_str());
    FUNLOG(Info, "config: download_proxy_http=%s", g_config.m_download_proxy_http.c_str());
    FUNLOG(Info, "config: download_proxy_https=%s", g_config.m_download_proxy_https.c_str());

    char env[64] = {0};
    sprintf(env, "CUDA_VISIBLE_DEVICES=%d", g_device_num);
    if (putenv(env) != 0) {
        FUNLOG(Err, "add env params %s fail!", env);
    } else {
        FUNLOG(Info, "add env params %s success!", env);
    }

    if (g_attr_model == NULL) {
        FUNLOG(Info, "try to new attr_model...");
        yyai::conf_param_t params;
        params.gpu_id = 0;
        params.model_pool_size = g_pool_size;
        params.models_dir = g_model_dir;
        g_attr_model = new yyai::CoverQualityModelsApi(params);
    }

    //StaticService::getInstance()->init(g_config.m_staticservers);
    PushService::getInstance()->init(g_config.m_push_threads);

    RabbitMQClient client(g_config.m_rabbit_host, g_config.m_rabbit_port, g_config.m_rabbit_user, g_config.m_rabbit_pwd,
                          g_config.m_rabbit_vhost, g_config.m_rabbit_queue, &callback_func, g_worker_threads_num);
    printf("start to consum....\n");
    FUNLOG(Info, "start to consum....");
    client.consum();

    //after threads die,then clean up curl
    curl_global_cleanup();
    return 0;
}
