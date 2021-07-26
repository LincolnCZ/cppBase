#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>

extern int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, AVCodecContext **codec_ctx, enum AVMediaType type);

// copy from ffmpeg example

/** Decode one audio frame from the input file. */
extern int decode_audio_frame(int stream_index,
                              AVFormatContext *input_format_context,
                              AVCodecContext *input_codec_context,
                              AVFrame ***frames,
                              int *data_present, int *finished);

extern void free_frame_list(AVFrame **frames, int size);

/** Encode one frame worth of audio to the output file. */
extern int encode_audio_frame(AVFrame *frame,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *data_present);

extern int init_resampler(AVCodecParameters *input_codec_context,
                          AVCodecParameters *output_codec_context,
                          SwrContext **resample_context);

extern int convert_and_store(AVFrame *input_frame,
                             AVAudioFifo *fifo,
                             AVCodecContext *output_codec_context,
                             SwrContext *resampler_context);

extern int init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size);

#ifdef __cplusplus
}
#endif