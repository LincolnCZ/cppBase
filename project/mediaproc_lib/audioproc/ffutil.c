#include "ffutil.h"
#include "core/logger.h"
#include "core/ffmpeg_common.h"
#include <assert.h>

static void init_packet(AVPacket *packet) {
    av_init_packet(packet);
    /** Set the packet data and size so that it is recognized as being empty. */
    packet->data = NULL;
    packet->size = 0;
}

int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, AVCodecContext **codec_ctx, enum AVMediaType type) {
    AVCodecContext *avctx = NULL;
    AVCodec *codec = NULL;
    int stream_index = -1;
    AVStream *st = find_stream(fmt_ctx, type);
    if (st == NULL) {
        FUNLOG(Warn, "Could not find %s stream", av_get_media_type_string(type));
        return -1;
    }

    stream_index = st->index;

    /** Find a decoder for the audio stream. */
    codec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!codec) {
        FUNLOG(Warn, "Failed to find %s codec", av_get_media_type_string(type));
        return AVERROR(EINVAL);
    }

    /** allocate a new decoding context */
    avctx = avcodec_alloc_context3(codec);
    if (!avctx) {
        FUNLOG(Warn, "Failed to alloc AVCodecContext");
        return AVERROR(ENOMEM);
    }

    /** initialize the stream parameters with demuxer information */
    int error = avcodec_parameters_to_context(avctx, st->codecpar);
    if (error < 0) {
        FUNLOG(Warn, "Failed to init AVCodecContext parameters, error %d", error);
        avcodec_free_context(&avctx);
        return error;
    }

    /** Open the decoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, codec, NULL)) < 0) {
        FUNLOG(Warn, "Failed to init AVCodecContext parameters, error %d", error);
        avcodec_free_context(&avctx);
        return error;
    }
    *stream_idx = stream_index;
    *codec_ctx = avctx;
    return 0;
}

int decode_audio_frame(int stream_index,
                       AVFormatContext *input_format_context,
                       AVCodecContext *input_codec_context,
                       AVFrame ***frames,
                       int *data_present, int *finished) {
    int result = 0;
    int error = 0;
    *frames = NULL;
    *data_present = 0;
    *finished = 0;

    /** Packet used for temporary storage. */
    AVPacket input_packet;
    init_packet(&input_packet);

    /** Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
        if (error == AVERROR_EOF) {
            *finished = 1;
            goto end;
        } else {
            FUNLOG(Error, "Could not read frame (error %d)", error);
            result = error;
            goto end;
        }
    }
    if (input_packet.stream_index != stream_index) {
        *finished = 0;
        *data_present = 0;
        goto end;
    }

    /* send the packet with the compressed data to the decoder */
    error = avcodec_send_packet(input_codec_context, &input_packet);
    if (error < 0) {
        // 如果已完成，或者是mp3并且是无效数据，则直接返回完成
        if (error == AVERROR_EOF ||
            (error == AVERROR_INVALIDDATA && input_codec_context->codec_id == AV_CODEC_ID_MP3)) {
            *finished = 1;
        } else {
            FUNLOG(Error, "Could not send packet (error %d)", error);
            result = error;
        }
        goto end;
    }

    /* read all the output frames (in general there may be any number of them */
    int frame_capacity = 8;
    int frame_size = 0;
    AVFrame **frame_buffer = malloc(sizeof(AVFrame *) * frame_capacity);

    while (error >= 0) {
        AVFrame *frame = av_frame_alloc();
        error = avcodec_receive_frame(input_codec_context, frame);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            av_frame_free(&frame);
            break;
        } else if (error < 0) {
            FUNLOG(Error, "Could not recive frame (error %d)", error);
            av_frame_free(&frame);
            result = error;
            break;
        }

        // push frame
        if (frame_size + 1 >= frame_capacity) {
            frame_capacity = frame_capacity * 2;
            frame_buffer = realloc(frame_buffer, sizeof(AVFrame *) * frame_capacity);
        }
        frame_buffer[frame_size] = frame;
        frame_size++;
    }
    *frames = frame_buffer;
    *data_present = frame_size;

    end:
    /**
     * If the decoder has not been flushed completely, we are not finished,
     * so that this function has to be called again.
     */
    if (*finished && *data_present > 0)
        *finished = 0;
    av_packet_unref(&input_packet);
    return result;
}

extern void free_frame_list(AVFrame **frames, int size) {
    if (frames == NULL)
        return;
    for (int i = 0; i < size; i++) {
        av_frame_free(frames + i);
    }
    free(frames);
}

int encode_audio_frame(AVFrame *frame,
                       AVFormatContext *output_format_context,
                       AVCodecContext *output_codec_context,
                       int *data_present) {
    int error;

    /* send the frame for encoding */
    error = avcodec_send_frame(output_codec_context, frame);
    if (error < 0) {
        FUNLOG(Error, "Error sending the frame to the encoder (error %d)", error);
        return error;
    }

    /* read all the available output packets (in general there may be any number of them) */
    while (1) {
        AVPacket output_packet;
        init_packet(&output_packet);

        error = avcodec_receive_packet(output_codec_context, &output_packet);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
            av_packet_unref(&output_packet);
            break;
        } else if (error < 0) {
            FUNLOG(Error, "Error encoding audio frame (error %d)", error);
            av_packet_unref(&output_packet);
            return error;
        }

        if ((error = av_write_frame(output_format_context, &output_packet)) < 0) {
            FUNLOG(Error, "Could not write frame (error %d)", error);
            av_packet_unref(&output_packet);
            return error;
        }
        av_packet_unref(&output_packet);
    }
    return 0;
}

/**
 * Initialize the audio resampler based on the input and output codec settings.
 * If the input and output sample formats differ, a conversion is required
 * libswresample takes care of this, but requires initialization.
 */
int init_resampler(AVCodecParameters *input_codec_context,
                   AVCodecParameters *output_codec_context,
                   SwrContext **resample_context) {
    int error;

    /**
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    *resample_context = swr_alloc_set_opts(NULL,
                                           av_get_default_channel_layout(output_codec_context->channels),
                                           output_codec_context->format,
                                           output_codec_context->sample_rate,
                                           av_get_default_channel_layout(input_codec_context->channels),
                                           input_codec_context->format,
                                           input_codec_context->sample_rate,
                                           0, NULL);
    if (!*resample_context) {
        FUNLOG(Error, "Could not allocate resample context");
        return AVERROR(ENOMEM);
    }

    /** Open the resampler with the specified parameters. */
    if ((error = swr_init(*resample_context)) < 0) {
        FUNLOG(Error, "Could not open resample context");
        swr_free(resample_context);
        return error;
    }
    return 0;
}

int convert_and_store(AVFrame *input_frame,
                      AVAudioFifo *fifo,
                      AVCodecContext *output_codec_context,
                      SwrContext *resampler_context) {
    uint8_t *converted_input_samples[8];
    int ret = AVERROR_EXIT;
    int error = 0;

    /** Initialize the temporary storage for the converted input samples. */
    memset(converted_input_samples, 0, sizeof(converted_input_samples));
    int64_t out_nb_samples = swr_get_out_samples(resampler_context, input_frame->nb_samples);
    if ((error = av_samples_alloc(converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  out_nb_samples,
                                  output_codec_context->sample_fmt, 1)) < 0) {
        FUNLOG(Error, "Could not allocate converted input samples (error %d)", error);
        av_freep(&converted_input_samples[0]);
        goto cleanup;
    }

    /**
     * Convert the input samples to the desired output sample format.
     * This requires a temporary storage provided by converted_input_samples.
     */
    error = swr_convert(resampler_context,
                        converted_input_samples, out_nb_samples,
                        (const uint8_t **) input_frame->extended_data, input_frame->nb_samples);
    if (error < 0) {
        FUNLOG(Error, "Could not convert input samples (error %d)", error);
        goto cleanup;
    } else if (error == 0) {
        ret = 0;
        goto cleanup;
    }
    int nb_sample = error;

    /** Add the converted input samples to the FIFO buffer for later processing. */
    if (av_audio_fifo_write(fifo, (void **) converted_input_samples, nb_sample) < nb_sample) {
        FUNLOG(Error, "Could not write data to FIFO");
        goto cleanup;
    }
    ret = 0;

    cleanup:
    av_freep(&converted_input_samples[0]);
    return ret;
}

int init_output_frame(AVFrame **frame,
                      AVCodecContext *output_codec_context,
                      int frame_size) {
    int error;

    /** Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        FUNLOG(Error, "Could not allocate output frame");
        return AVERROR_EXIT;
    }

    /**
     * Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity.
     */
    (*frame)->nb_samples = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format = output_codec_context->sample_fmt;
    (*frame)->sample_rate = output_codec_context->sample_rate;

    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        FUNLOG(Error, "Could allocate output frame samples");
        av_frame_free(frame);
        return error;
    }

    return 0;
}
