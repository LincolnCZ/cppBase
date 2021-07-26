#include <memory>
#include <assert.h>
#include "core/logger.h"
#include "core/utils.h"
#include "core/ffmpeg_common.h"
#include "core/AVIOMemory.h"
#include "ffutil.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "AudioEncode.h"
#include "AudioProcessor.h"

static bool checkFormat(const char *format, const char *dest) {
    const char *sfind = strstr(format, dest);
    if (sfind == NULL)
        return false;
    if (sfind != format && *(sfind - 1) != ',')
        return false;
    return true;
}

struct AudioProcessor::AudioProcessorImpl {
    AVFormatContext *fmtCtx;
    AVIOReadMemory *readMemory;
    double duration;    // 解码获取的准确时长
};

AudioProcessor::AudioProcessor() {
    initFFmpeg();

    _impl = new(AudioProcessorImpl);
    _impl->fmtCtx = NULL;
    _impl->readMemory = NULL;
    _impl->duration = 0.0;
}

AudioProcessor::~AudioProcessor() {
    if (_impl->fmtCtx)
        avformat_close_input(&_impl->fmtCtx);
    if (_impl->readMemory)
        delete _impl->readMemory;
    delete _impl;
}

bool AudioProcessor::init(const char *taskId, const std::string &input, bool inputPath) {
    assert(_impl->fmtCtx == NULL);

    // 打开输入文件信息
    AVFormatContext *fmtCtx = avformat_alloc_context();
    if (inputPath) {
        if (avformat_open_input(&fmtCtx, input.c_str(), NULL, NULL) != 0) {
            avformat_free_context(fmtCtx);
            FUNLOG(Warn, "%s, open avformat fail", taskId);
            return false;
        }
    } else {
        _impl->readMemory = new AVIOReadMemory(input.data(), input.size());
        fmtCtx->pb = _impl->readMemory->get();
        fmtCtx->flags = AVFMT_FLAG_CUSTOM_IO;
        if (avformat_open_input(&fmtCtx, NULL, NULL, NULL) != 0) {
            avformat_free_context(fmtCtx);
            FUNLOG(Warn, "%s, open avformat fail", taskId);
            return false;
        }
    }
    _impl->fmtCtx = fmtCtx;

    // 打开查找音频流
    if (avformat_find_stream_info(fmtCtx, NULL) < 0) {
        FUNLOG(Warn, "%s, find stream info fail", taskId);
        return false;
    }
    AVStream *audio_stream = find_stream(fmtCtx, AVMEDIA_TYPE_AUDIO);
    if (audio_stream == NULL) {
        FUNLOG(Warn, "%s, find audio stream fail", taskId);
        return false;
    }
    // av_dump_format(fmt_ctx, stream_in->index, "input", 0);
    return true;
}

const char *AudioProcessor::formatName() const {
    assert(_impl->fmtCtx != NULL);
    return _impl->fmtCtx->iformat->name;
}

double AudioProcessor::duration() const {
    if (_impl->duration > 0.0) {
        return _impl->duration;
    }
    return double(_impl->fmtCtx->duration) / double(AV_TIME_BASE);
}

double AudioProcessor::calcDuration() {
    if (_impl->duration > 0.0) {
        return _impl->duration;
    }
    // aac格式的时长判断存在问题，需要通过解码获取准确长度
    if (checkFormat(formatName(), "aac")) {
        std::string tmp;
        decodeAudio(16000, 1, tmp);
        avformat_seek_file(_impl->fmtCtx, -1, 0, 0, 0, AVSEEK_FLAG_BYTE);
        return _impl->duration;
    }
    return double(_impl->fmtCtx->duration) / double(AV_TIME_BASE);
}

bool AudioProcessor::decodeAudio(int sampleRate, int channels, std::string &outbuf) {
    AVFormatContext *fmtCtx = _impl->fmtCtx;
    int stream_index = -1;
    AVCodecContext *codec_in = NULL;
    if (open_codec_context(&stream_index, fmtCtx, &codec_in, AVMEDIA_TYPE_AUDIO) != 0) {
        return false;
    }
    auto _defer_codec = utility::defer([&codec_in]() { avcodec_free_context(&codec_in); });

    //设置转码相关参数
    const uint64_t in_channel_layout = av_get_default_channel_layout(codec_in->channels);
    const enum AVSampleFormat in_sample_fmt = codec_in->sample_fmt;
    const int in_sample_rate = codec_in->sample_rate;
    const enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    const int out_sample_rate = sampleRate; //采样率
    const int out_channels = channels;
    const uint64_t out_channel_layout = av_get_default_channel_layout(channels);

    //初始化转码器
    struct SwrContext *convert_ctx = swr_alloc();
    convert_ctx = swr_alloc_set_opts(convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                     in_channel_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    swr_init(convert_ctx);

    bool result = true;
    int64_t sample_out_size = 0;
    outbuf.clear();

    //创建buffer
    int max_out_nb_samples = 0;
    uint8_t *out_buffer[8];
    int out_linesize = 0;
    memset(out_buffer, 0, sizeof(out_buffer));

    while (result) {
        AVFrame **frame_list = NULL;
        int data_present = 0, finished = 0;
        auto _defer_codec = utility::defer(
                [&frame_list, &data_present]() { free_frame_list(frame_list, data_present); });

        //解码声音
        if (decode_audio_frame(stream_index, fmtCtx, codec_in, &frame_list, &data_present, &finished)) {
            result = false;
            break;
        }
        if (finished && !data_present) {
            break;
        }

        for (int i = 0; i < data_present; i++) {
            AVFrame *frame = frame_list[i];

            // 申请输出buffer
            int64_t out_nb_samples = av_rescale_rnd(frame->nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);
            if (frame->nb_samples > max_out_nb_samples) {
                av_freep(&out_buffer[0]);
                int r = av_samples_alloc(out_buffer, &out_linesize, out_channels, out_nb_samples, out_sample_fmt, 1);
                if (r < 0) {
                    LOG(Warn, "alloc output buffer fail");
                    result = false;
                    break;
                }
                max_out_nb_samples = frame->nb_samples;
            }

            //转码
            int ret_sample = swr_convert(convert_ctx, out_buffer, out_nb_samples, (const uint8_t **) frame->data,
                                         frame->nb_samples);
            if (ret_sample < 0) {
                LOG(Warn, "swr convert fail");
                result = false;
                break;
            }
            if (ret_sample == 0)
                continue;
            int buffer_size = av_samples_get_buffer_size(NULL, out_channels, ret_sample, out_sample_fmt, 1);
            outbuf.append((char *) out_buffer[0], buffer_size);
            sample_out_size += ret_sample;
        }
    }
    av_freep(&out_buffer[0]);
    swr_free(&convert_ctx);

    if (result) {
        _impl->duration = double(sample_out_size) / double(out_sample_rate);
    }
    return result;
}

bool AudioProcessor::convertAudio(int sampleRate, int channels, const std::string &format, std::string &outbuf) {
    AVFormatContext *fmtCtx = _impl->fmtCtx;
    int stream_index = -1;
    AVCodecContext *codec_in = NULL;
    if (open_codec_context(&stream_index, fmtCtx, &codec_in, AVMEDIA_TYPE_AUDIO) != 0) {
        FUNLOG(Warn, "open input codec fail");
        return false;
    }
    auto _defer_codec = utility::defer([&codec_in]() { avcodec_free_context(&codec_in); });
    AVStream *stream_in = fmtCtx->streams[stream_index];

    // 初始化写入数据
    std::unique_ptr<AudioEncode> fmt_out(new AudioEncode);
    if (!fmt_out->init(stream_in->codecpar, format.c_str(), sampleRate, channels)) {
        FUNLOG(Warn, "init output format fail");
        return false;
    }

    bool result = true;
    while (result) {
        AVFrame **frame_list = NULL;
        int data_present = 0, finished = 0;
        auto _defer_codec = utility::defer(
                [&frame_list, &data_present]() { free_frame_list(frame_list, data_present); });

        //解码声音
        if (decode_audio_frame(stream_index, fmtCtx, codec_in, &frame_list, &data_present, &finished)) {
            result = false;
            break;
        }
        if (finished && !data_present) {
            break;
        }

        for (int i = 0; i < data_present; i++) {
            AVFrame *frame = frame_list[i];
            if (!fmt_out->write(frame)) {
                result = false;
                FUNLOG(Warn, "write output fail, pts %ld", frame->pts);
                break;
            }
        }
    }

    if (result) {
        _impl->duration = double(fmt_out->pts()) / double(sampleRate);
        fmt_out->writeTrailer();
        outbuf = fmt_out->get();
    }
    return result;
}

static bool splitAudioDecode(AVFormatContext *fmt_ctx, std::vector<std::string> &outbuf, int segtime) {
    int stream_index = -1;
    AVCodecContext *codec_in = NULL;
    if (open_codec_context(&stream_index, fmt_ctx, &codec_in, AVMEDIA_TYPE_AUDIO) != 0) {
        FUNLOG(Warn, "open input codec fail");
        return false;
    }
    auto _defer_codec = utility::defer([&codec_in]() { avcodec_free_context(&codec_in); });
    AVStream *stream_in = fmt_ctx->streams[stream_index];

    // 初始化写入数据
    std::unique_ptr<AudioEncode> fmt_out(new AudioEncode);
    if (!fmt_out->init(stream_in->codecpar, "mp4", stream_in->codecpar->sample_rate, stream_in->codecpar->channels)) {
        FUNLOG(Warn, "init output format fail");
        return false;
    }

    bool result = true;
    int64_t base_pts = 0;
    while (result) {
        AVFrame **frame_list = NULL;
        int data_present = 0, finished = 0;
        auto _defer_codec = utility::defer(
                [&frame_list, &data_present]() { free_frame_list(frame_list, data_present); });

        //解码声音
        if (decode_audio_frame(stream_index, fmt_ctx, codec_in, &frame_list, &data_present, &finished)) {
            result = false;
            break;
        }
        if (finished && !data_present) {
            break;
        }

        for (int i = 0; i < data_present; i++) {
            AVFrame *frame = frame_list[i];

            // 检查音频分割输出
            int64_t bias_pts = frame->pts - base_pts;
            if (compare_timestamp(bias_pts, stream_in->time_base, segtime) >= 0) {
                fmt_out->writeTrailer();
                outbuf.push_back(fmt_out->get());

                fmt_out.reset(new AudioEncode);
                if (!fmt_out->init(stream_in->codecpar, "mp4", stream_in->codecpar->sample_rate,
                                   stream_in->codecpar->channels)) {
                    result = false;
                    FUNLOG(Warn, "reset output format fail, pts %ld", frame->pts);
                    break;
                }
                base_pts = frame->pts;
                bias_pts = 0;
            }

            if (!fmt_out->write(frame)) {
                result = false;
                FUNLOG(Warn, "write output fail, pts %ld", frame->pts);
                break;
            }
        }
    }

    if (result) {
        fmt_out->writeTrailer();
        outbuf.push_back(fmt_out->get());
    }
    return result;
}

bool AudioProcessor::splitAudio(std::vector<std::string> &outbuf, int segtime) {
    outbuf.clear();
    return splitAudioDecode(_impl->fmtCtx, outbuf, segtime);
}