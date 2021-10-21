// tutorial03.c
// A pedagogical video player that will stream through every video frame as fast as it can
// and play audio (out of sync).
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard, 
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
//
// Updates tested on:
// Mac OS X 10.11.6
// Apple LLVM version 8.0.0 (clang-800.0.38)
//
// Use 
//
// $ gcc -o tutorial03 tutorial03.c -lavutil -lavformat -lavcodec -lswscale -lz -lm `sdl-config --cflags --libs`
//
// to build (assuming libavutil/libavformat/libavcodec/libswscale are correctly installed your system).
//
// Run using
//
// $ tutorial03 myvideofile.mpg
//
// to play the stream on your screen with voice.

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <SDL2/SDL.h>
#include <SDL_thread.h>
}

#include <cstdio>
#include <iostream>

using namespace std;

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

PacketQueue audioq;

int quit = 0;

void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    if (av_packet_ref(pkt, pkt) < 0) {
        return -1;
    }
    pkt1 = (AVPacketList *) av_malloc(sizeof(AVPacketList));
    if (!pkt1) {
        return -1;
    }
    pkt1->pkt = *pkt;
    pkt1->next = NULL;


    SDL_LockMutex(q->mutex);

    if (!q->last_pkt) {
        q->first_pkt = pkt1;
    } else {
        q->last_pkt->next = pkt1;
    }
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
    return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (quit) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt) {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;

    int len1, data_size = 0;

    for (;;) {
        while (audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
            if (len1 < 0) {
                // if error, skip frame.
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            if (got_frame) {
                data_size = av_samples_get_buffer_size(NULL, aCodecCtx->channels, frame.nb_samples,
                                                       aCodecCtx->sample_fmt, 1);
                memcpy(audio_buf, frame.data[0], data_size);
            }
            if (data_size <= 0) {
                // No data yet, get more frames.
                continue;
            }
            // We have data, return it and come back for more later.
            return data_size;
        }
        if (pkt.data) {
            av_packet_unref(&pkt);
        }

        if (quit) {
            return -1;
        }

        if (packet_queue_get(&audioq, &pkt, 1) < 0) {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
    AVCodecContext *aCodecCtx = (AVCodecContext *) userdata;
    int len1, audio_size;

    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while (len > 0) {
        if (audio_buf_index >= audio_buf_size) {
            // We have already sent all our data; get more.
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, audio_buf_size);
            if (audio_size < 0) {
                // If error, output silence.
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;

//        SDL_Delay(10);
    }
}

//SDL
SDL_Renderer *g_sdl_renderer;
SDL_Texture *g_sdl_texture;
SDL_Window *g_sdl_window;
SDL_Event g_sdl_event;

int init_sdl(int width, int height, AVCodecContext *aCodecCtx) {
    cout << "init sdl, width:" << width << ",height:" << height << endl;
    // 初始化sdl
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { // 目前只需要播放视频
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }
    // 创建窗体
    g_sdl_window = SDL_CreateWindow("FFmpeg+SDL播放视频", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    width, height, SDL_WINDOW_ALLOW_HIGHDPI);
    if (g_sdl_window == NULL) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return -1;
    }
    // 从窗体创建渲染器
    g_sdl_renderer = SDL_CreateRenderer(g_sdl_window, -1, 0);
    if (!g_sdl_renderer) {
        cout << "create g_sdl_renderer failed" << endl;
        return -1;
    }
    // 创建渲染器纹理，我的视频编码格式是SDL_PIXELFORMAT_IYUV，可以从FFmpeg探测到的实际结果设置颜色格式
    g_sdl_texture = SDL_CreateTexture(g_sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!g_sdl_texture) {
        cout << "create g_sdl_texture failed;" << endl;
        return -1;
    }

    SDL_AudioSpec wanted_spec, spec;

    // Set audio settings from codec info.
    wanted_spec.freq = aCodecCtx->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = aCodecCtx->channels;
//    wanted_spec.channels = 2;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = aCodecCtx;

    cout << "freq :" << wanted_spec.freq << ",channels:" << aCodecCtx->channels << ", format:" << wanted_spec.format
         << ",samples:" << wanted_spec.samples << endl;


//    SDL_AudioDeviceID deviceID;
//    if ((deviceID = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2) {
//        cout << "SDL_OpenAudioDevice with error deviceID : " << deviceID << endl;
//        return -1;
//    }
//    SDL_PauseAudioDevice(deviceID, 0);

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }

    SDL_PauseAudio(0);
    return 0;
}

int free_sdl() {
    SDL_DestroyRenderer(g_sdl_renderer);
    SDL_DestroyTexture(g_sdl_texture);
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;
    int i, videoStream, audioStream;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameYUV = NULL;
    AVPacket packet;
    int frameFinished;

    AVCodecContext *aCodecCtx = NULL;
    AVCodec *aCodec = NULL;

    struct SwsContext *sws_ctx = NULL;
    AVDictionary *videoOptionsDict = NULL;
    AVDictionary *audioOptionsDict = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: test <file>\n");
        exit(1);
    }
    // Register all formats and codecs.
    av_register_all();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    // Open video file.
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
        return -1; // Couldn't open file.
    }

    // Retrieve stream information.
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -1; // Couldn't find stream information.
    }

    // Dump information about file onto standard error.
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream.
    videoStream = -1;
    audioStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {
            videoStream = i;
        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0) {
            audioStream = i;
        }
    }
    if (videoStream == -1) {
        return -1; // Didn't find a video stream.
    }
    if (audioStream == -1) {
        return -1;
    }

    aCodecCtx = pFormatCtx->streams[audioStream]->codec;
    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (!aCodec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    avcodec_open2(aCodecCtx, aCodec, &audioOptionsDict);

    // audio_st = pFormatCtx->streams[index].
    packet_queue_init(&audioq);

    // Get a pointer to the codec context for the video stream.
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream.
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found.
    }
    // Open codec.
    if (avcodec_open2(pCodecCtx, pCodec, &videoOptionsDict) < 0)
        return -1; // Could not open codec.

    // Allocate video frame.
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    unsigned char *out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // init sdl
    if (init_sdl(pCodecCtx->width, pCodecCtx->height, aCodecCtx)) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    // Allocate a place to put our YUV image on that screen.
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
                             pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);


    // Read frames and save first five frames to disk.
    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame.
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if (frameFinished) {

                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //SDL---------------------------
                SDL_UpdateTexture(g_sdl_texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(g_sdl_renderer);
                SDL_RenderCopy(g_sdl_renderer, g_sdl_texture, NULL, NULL);
                SDL_RenderPresent(g_sdl_renderer);
                SDL_Delay(40); // 防止显示过快，如果要实现倍速播放，只需要调整delay时间就可以了。
                //SDL End-----------------------

                av_packet_unref(&packet);
            }
        } else if (packet.stream_index == audioStream) {
            packet_queue_put(&audioq, &packet);
        } else {
            // Free the packet that was allocated by av_read_frame.
            av_packet_unref(&packet);
        }

        SDL_PollEvent(&g_sdl_event);
        switch (g_sdl_event.type) {
            case SDL_QUIT:
                quit = 1;
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
    }

    // Free the YUV frame.
    av_free(pFrame);

    // Close the codec.
    avcodec_close(pCodecCtx);

    // Close the video file.
    avformat_close_input(&pFormatCtx);

    free_sdl();

    return 0;
}