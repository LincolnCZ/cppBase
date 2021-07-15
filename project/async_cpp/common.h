#ifndef _COMMON_H
#define _COMMON_H

#include<string>
using std::string;

enum PROCESS_ERROR
{
	PROCESS_ERROR_OK=0,
	PROCESS_ERROR_DOWNLOAD=1,
	PROCESS_ERROR_DOWNLOAD_CHECK=2,
	PROCESS_ERROR_PREDICT=3,
	PROCESS_ERROR_CREATE_DIR=4,
	PROCESS_ERROR_PROCE_ONE_FILE=5,
	PROCESS_ERROR_NO_PUSH_CONFIG=6,
	PROCESS_ERROR_PUSH_RESULT_FAILED=7,
	PROCESS_ERROR_PUSH_FEEDBACK_FAILED=8,
	PROCESS_ERROR_UPLOADFILE=9,
	PROCESS_ERROR_INIT_MODEL=10,
    PROCESS_ERROR_TASK_PARAMS=11,
	PROCESS_ERROR_EXCEPTION=12,
	PROCESS_ERROR_PROCESS_TASK=100,
};

const int MAX_INPUT_SIZE = 10;

//所有业务请求
const string ALL_BUSINESS_PROCESS_REQUEST = "all_business_process_request";
const string ALL_BUSINESS_PROCESS_ONE_FIEL = "all_business_process_oen_file";
// 以下使用secretid，区分不同业务的请求
const string PROCESS_REQUEST = "/process_request";
const string DATALIST_TOO_LARGE = "/len_of_dalalist_too_large";
const string PRIORITY = "/priority";
const string PROCESS_ONE_FILE = "/process_oen_file";
const string DOWNLOAD_FIEL = "/download_file";
const string DOWNLOAD_FIEL_CHECK = "/download_file_check";
const string ALGORITHM_PRDICT = "/algorithm_predict";
const string ALGORITHM_PRDICT_GET_FRAME = "/predict_get_frame";
const string ALGORITHM_PRDICT_GET_WAV = "/predict_get_wav";
const string UPLOAD_FILE_BS2 = "/upload_file/bs2";
const string UPLOAD_FILE_GOOGLE = "/upload_file/google";
const string UPLOAD_FILE_OSS = "/upload_file/oss";
const string PUBLISH_RESULT_HTTP = "/publish_result/http";
const string PUBLISH_RESULT_KAFKA = "/publish_result/kafka";
const string NO_PUSH_CONFIG = "/no_push_config";
const string FEEDBACK_PUBLISH = "/feedback_publish";
const string VIDEO_DURATION = "/video_duration";

#endif
