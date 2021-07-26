#include "AudioEncode.h"
#include "core/logger.h"
#include "core/utils.h"
#include "ffutil.h"

bool checkFormat(const char *format)
{
    if(strcmp(format, "mp4") == 0) {
        return true;
    } else if(strcmp(format, "adts") == 0) {
        return true;
    }
    return false;
}

AudioEncode::AudioEncode()
{
    _fmt = NULL;
    _codec_out = NULL;
    _fifo = NULL;
    _resampler = NULL;
    _pts = 0;
}

AudioEncode::~AudioEncode()
{
    if(_resampler)
        swr_free(&_resampler);
    if(_fifo)
        av_audio_fifo_free(_fifo);
    if(_codec_out)
        avcodec_free_context(&_codec_out);
    if(_fmt)
        avformat_free_context(_fmt);
}

bool AudioEncode::init(AVCodecParameters *codec_in, const char *format, int sample_rate, int channels)
{
    if(!checkFormat(format)) {
        FUNLOG(Error, "check format %s fail", format);
        return false;
    }
    AVFormatContext *fmt = NULL;
    if(avformat_alloc_output_context2(&fmt, NULL, format, NULL) < 0) {
        FUNLOG(Error, "alloc output context fail");
        return false;
    }
    fmt->pb = _avio.get();
    fmt->flags = AVFMT_FLAG_CUSTOM_IO;
    _fmt = fmt;

    // 设置输出音频流
    AVCodec *codec = avcodec_find_encoder_by_name("libfdk_aac");
    AVStream *stream_out = avformat_new_stream(_fmt, NULL);
    if(stream_out == NULL) {
        FUNLOG(Error, "new stream out fail");
        return false;
    }
    stream_out->time_base.num = 1;
    stream_out->time_base.den = sample_rate;

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->channels = channels;
    codec_ctx->channel_layout = av_get_default_channel_layout(codec_ctx->channels);
    codec_ctx->sample_rate = sample_rate;
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    codec_ctx->bit_rate = 96000;
    codec_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    /* Some container formats (like MP4) require global headers to be present.
     * Mark the encoder so that it behaves accordingly. */
    if (fmt->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if(avcodec_open2(codec_ctx, codec, NULL) < 0) {
        FUNLOG(Error, "Failed to open encoder");
        return false;
    }
    _codec_out = codec_ctx;

    if(avcodec_parameters_from_context(stream_out->codecpar, _codec_out) < 0) {
        FUNLOG(Error, "Could not initialize stream parameters");
        return false;
    }
    // av_dump_format(_fmt, 0, "output", 1);

    _fifo = av_audio_fifo_alloc(codec_ctx->sample_fmt, codec_ctx->channels, 1);
    if(_fifo == NULL) {
        FUNLOG(Error, "Failed to alloc audio fifo");
        return false;
    }

    if(init_resampler(codec_in, stream_out->codecpar, &_resampler)) {
        FUNLOG(Error, "Failed to init resampler");
        return false;
    }

    if(avformat_write_header(_fmt, NULL) < 0) {
        FUNLOG(Error, "audio format out write head fail");
        return false;
    }
    return true;
}

bool AudioEncode::write(AVFrame *frame)
{
    const int output_frame_size = _codec_out->frame_size;

    // 把数据写入缓冲区后读取
    if(frame) {
        if(convert_and_store(frame, _fifo, _codec_out, _resampler)) {
            FUNLOG(Error, "convert and store fail");
            return false;
        }
    }

    // 读取数据并编码
    while (av_audio_fifo_size(_fifo) >= output_frame_size) {
        AVFrame *output_frame;
        const int frame_size = std::min(av_audio_fifo_size(_fifo), output_frame_size);
        if(init_output_frame(&output_frame, _codec_out, frame_size)) {
            FUNLOG(Error, "init output frame fail");
            return false;
        }
        auto _defer_frame = utility::defer([&output_frame]() { av_frame_free(&output_frame); });

        if(av_audio_fifo_read(_fifo, (void **)output_frame->data, frame_size) < frame_size) {
            FUNLOG(Error, "Could not read data from FIFO");
            return false;
        }
        int data_written;
        output_frame->pts = _pts;
        _pts += output_frame->nb_samples;
        if(encode_audio_frame(output_frame, _fmt, _codec_out, &data_written)) {
            FUNLOG(Error, "Could not encode audio");
            return false;
        }
    }
    return true;
}

bool AudioEncode::writeTrailer()
{
    if(!flush_encoder()) {
        return false;
    }
    return av_write_trailer(_fmt) == 0;
}

bool AudioEncode::flush_encoder()
{
    while(av_audio_fifo_size(_fifo) > 0) {
        AVFrame *output_frame;
        const int frame_size = av_audio_fifo_size(_fifo);
        if(init_output_frame(&output_frame, _codec_out, frame_size)) {
            FUNLOG(Error, "init output frame fail");
            return false;
        }
        if(av_audio_fifo_read(_fifo, (void **)output_frame->data, frame_size) < 0) {
            FUNLOG(Error, "Could not read data from FIFO");
            av_frame_free(&output_frame);
            return false;
        }
        int data_written;
        output_frame->pts = _pts;
        _pts += output_frame->nb_samples;
        if(encode_audio_frame(output_frame, _fmt, _codec_out, &data_written)) {
            FUNLOG(Error, "Could not encode audio");
            av_frame_free(&output_frame);
            return false;
        }
        av_frame_free(&output_frame);
    }

    /** Flush the encoder as it may have delayed frames. */
    int data_written = 0;
    do {
        if(encode_audio_frame(NULL, _fmt, _codec_out, &data_written)) {
            FUNLOG(Error, "Could not flush audio");
            return false;
        }
    } while(data_written);
    return true;
}

