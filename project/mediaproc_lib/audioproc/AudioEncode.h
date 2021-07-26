#pragma once
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}
#include "core/AVIOMemory.h"

class AudioEncode
{
public:
    AudioEncode();
    ~AudioEncode();
    AudioEncode(const AudioEncode&) = delete;
    AudioEncode &operator=(const AudioEncode&) = delete;

    bool init(AVCodecParameters *codec_in, const char *format, int sample_rate, int channels);
    bool write(AVFrame *frame);
    bool writeTrailer();
    const std::string &get() {return _avio.getBuffer(); }
    int64_t pts() const {return _pts;}

private:
    bool flush_encoder();

private:
    AVIOMemory _avio;
    AVFormatContext *_fmt;
    AVCodecContext *_codec_out;
    AVAudioFifo *_fifo;
    SwrContext *_resampler;
    int64_t _pts;
};