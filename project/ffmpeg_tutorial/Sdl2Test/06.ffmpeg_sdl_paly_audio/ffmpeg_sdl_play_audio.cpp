/**
 * 最简单的基于FFmpeg的音频播放器 2
 * Simplest FFmpeg Audio Player 2
 *
 * 本程序实现了音频的解码和播放。
 * 是最简单的FFmpeg音频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 *
 * 该版本使用SDL 2.0替换了第一个版本中的SDL 1.0。
 * 注意：SDL 2.0中音频解码的API并无变化。唯一变化的地方在于
 * 其回调函数的中的Audio Buffer并没有完全初始化，需要手动初始化。
 * 本例子中即SDL_memset(stream, 0, len);
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
};

using namespace std;

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//Output PCM
#define OUTPUT_PCM 1
//Use SDL
#define USE_SDL 1

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static Uint8 *g_audio_chunk;
static Uint32 g_audio_len;
static Uint8 *g_audio_pos;

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len) {
    //SDL 2.0
    SDL_memset(stream, 0, len);
    if (g_audio_len == 0)
        return;

    len = (len > g_audio_len ? g_audio_len : len);    /*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream, g_audio_pos, len, SDL_MIX_MAXVOLUME);
    g_audio_pos += len;
    g_audio_len -= len;
    cout << "fill_audio len:" << len << endl;
}
//-----------------

int init_sdl(int out_sample_rate, int out_channels, int out_nb_samples, AVCodecContext *aCodecCtx) {
#if USE_SDL
    //Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = aCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
        printf("can't open audio.\n");
        return -1;
    }

    //Play
    SDL_PauseAudio(0);
#endif
    return 0;
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx;
    int i, audioStreamIndex;
    AVCodecContext *aCodecCtx;
    AVCodec *pCodec;
    AVPacket *packet;
    uint8_t *out_buffer;
    AVFrame *pFrame;
    int ret;
    int got_picture;
    int index = 0;
    int64_t in_channel_layout;
    struct SwrContext *au_convert_ctx;

    FILE *pFile = NULL;
//    char url[] = "xiaoqingge.mp3";
    char url[] = "/Users/linchengzhong/Desktop/xijinping.mp4";

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    //Open
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    // Dump valid information onto standard error
    av_dump_format(pFormatCtx, 0, url, false);

    // Find the first audio stream
    audioStreamIndex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }

    if (audioStreamIndex == -1) {
        printf("Didn't find a audio stream.\n");
        return -1;
    }

    // Get a pointer to the codec context for the audio stream
    aCodecCtx = pFormatCtx->streams[audioStreamIndex]->codec;

    // Find the decoder for the audio stream
    pCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }

    // Open codec
    if (avcodec_open2(aCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

#if OUTPUT_PCM
    pFile = fopen("output.pcm", "wb");
#endif

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    //Out Audio Param
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    //nb_samples: AAC-1024 MP3-1152
    int out_nb_samples = aCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = aCodecCtx->sample_rate;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    //Out Buffer Size
    int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
    cout << "out_buffer_size :" << out_buffer_size << endl;
    out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

    //FIX:Some Codec's Context Information is missing
    in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);

    //Swr
    au_convert_ctx = swr_alloc();
    au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                        in_channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
    swr_init(au_convert_ctx);

    // init sdl
    if (init_sdl(out_sample_rate, aCodecCtx->channels, out_nb_samples, aCodecCtx)) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    pFrame = av_frame_alloc();
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audioStreamIndex) {
            ret = avcodec_decode_audio4(aCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("Error in decoding audio frame.\n");
                return -1;
            }
            cout << "got_picture:" << got_picture << endl;
            if (got_picture > 0) {
                swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **) pFrame->data,
                            pFrame->nb_samples);
#if 1
                printf("index:%5d\t pts:%lld\t packet size:%d pFrame_nb_samples:%d\n",
                       index, packet->pts, packet->size, pFrame->nb_samples);
#endif

#if OUTPUT_PCM
                //Write PCM
                fwrite(out_buffer, 1, out_buffer_size, pFile);
#endif
                index++;
            }

#if USE_SDL
            while (g_audio_len > 0)//Wait until finish
                SDL_Delay(1);

            //Set audio buffer (PCM data)
            g_audio_chunk = (Uint8 *) out_buffer;
            //Audio buffer length
            g_audio_len = out_buffer_size;
            g_audio_pos = g_audio_chunk;

#endif
        }
        av_free_packet(packet);
    }

    swr_free(&au_convert_ctx);

#if USE_SDL
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
#endif

#if OUTPUT_PCM
    fclose(pFile);
#endif
    av_free(out_buffer);
    avcodec_close(aCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
