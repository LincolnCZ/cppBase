// tutorial02.c
// A pedagogical video player that will stream through every video frame as fast as it can.
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
// $ gcc -o tutorial02 tutorial02.c -lavutil -lavformat -lavcodec -lswscale -lz -lm `sdl-config --cflags --libs`
//
// to build (assuming libavutil/libavformat/libavcodec/libswscale are correctly installed your system).
//
// Run using
//
// $ tutorial02 myvideofile.mpg
//
// to play the video stream on your screen.

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <SDL.h>
#include <SDL_thread.h>
}

#include <cstdio>
#include <iostream>

using namespace std;

//SDL
SDL_Renderer *g_sdl_renderer;
SDL_Texture *g_sdl_texture;
SDL_Window *g_sdl_window;
SDL_Event g_sdl_event;

int init_sdl(int width, int height) {
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
    int i, videoStreamIndex;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameYUV = NULL;
    int frameFinished;
    AVDictionary *optionsDict = NULL;
    struct SwsContext *sws_ctx = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: test <file>\n");
        exit(1);
    }
    // Register all formats and codecs.
    av_register_all();

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
    videoStreamIndex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        return -1; // Didn't find a video stream.
    }

    // Get a pointer to the codec context for the video stream.
    AVCodecParameters *codecpar = pFormatCtx->streams[videoStreamIndex]->codecpar;
    // Find the decoder for the video stream.
    pCodec = avcodec_find_decoder(codecpar->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found.
    }
    // Copy context.
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_parameters_to_context(pCodecCtx, codecpar)) {
        fprintf(stderr, "Couldn't copy codec context");
        return -1; // Error copying codec context.
    }
//    // Get a pointer to the codec context for the video stream.
//    pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
//
//    // Find the decoder for the video stream.
//    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
//    if (pCodec == NULL) {
//        fprintf(stderr, "Unsupported codec!\n");
//        return -1; // Codec not found.
//    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &optionsDict) < 0) {
        return -1; // Could not open codec.
    }

    // Allocate video frame.
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    unsigned char *out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    AVPacket *packet = av_packet_alloc();

    // init sdl
    if (init_sdl(pCodecCtx->width, pCodecCtx->height)) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
                             pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    // Read frames and save first five frames to disk.
    i = 0;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        // Is this a packet from the video stream?.
        if (packet->stream_index == videoStreamIndex) {
            // Decode video frame.
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);

            // Did we get a video frame?
            if (frameFinished) {
                // Convert the image into YUV format that SDL uses.
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //SDL---------------------------
                SDL_UpdateTexture(g_sdl_texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(g_sdl_renderer);
                SDL_RenderCopy(g_sdl_renderer, g_sdl_texture, NULL, NULL);
                SDL_RenderPresent(g_sdl_renderer);
                SDL_Delay(40); // 防止显示过快，如果要实现倍速播放，只需要调整delay时间就可以了。
                //SDL End-----------------------
            }
        }

        // Free the packet that was allocated by av_read_frame.
        av_packet_unref(packet);

        SDL_PollEvent(&g_sdl_event);
        switch (g_sdl_event.type) {
            case SDL_QUIT:
                printf("SDL_QUIT\n");
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
    }

    // Free the YUV frame.
    av_free(pFrame);
    av_free(pFrameYUV);

    // Close the codec.
    avcodec_close(pCodecCtx);

    // Close the video file.
    avformat_close_input(&pFormatCtx);

    free_sdl();

    return 0;
}
