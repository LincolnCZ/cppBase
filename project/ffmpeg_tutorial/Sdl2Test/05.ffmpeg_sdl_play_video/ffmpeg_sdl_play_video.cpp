#include <iostream>
#include <map>
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL.h>
#include <libavutil/imgutils.h>
}
using namespace std;

static AVFormatContext *fmt_ctx = NULL;
static AVCodec *videoCodec = NULL;
static AVCodecContext *codec_ctx_video = NULL;
static int mWidth, mHeight;
static int index_video_stream = -1;

//SDL
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Window *sdl_window;

int init_sdl();

int free_sdl();

int main() {
    cout << "FFmpeg, 解封装" << endl;
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
    const char *source_url = "http://clips.vorwaerts-gmbh.de/big_buck_bunny.mp4";

    if (avformat_open_input(&fmt_ctx, source_url, NULL, NULL)) { // 打开媒体源，构建AVFormatContext
        cerr << "could not open source file:" << source_url << endl;
        exit(1);
    }

    // 找到所有流,初始化一些基本参数
    index_video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    mWidth = fmt_ctx->streams[index_video_stream]->codecpar->width;
    mHeight = fmt_ctx->streams[index_video_stream]->codecpar->height;

    // 找到并打开解码器
    videoCodec = avcodec_find_decoder(fmt_ctx->streams[index_video_stream]->codecpar->codec_id);
    codec_ctx_video = avcodec_alloc_context3(videoCodec); // 根据解码器类型分配解码器上下文内存
    avcodec_parameters_to_context(codec_ctx_video, fmt_ctx->streams[index_video_stream]->codecpar); // 拷贝参数
    codec_ctx_video->thread_count = 1; // 解码线程数量
    cout << "thread_count = " << codec_ctx_video->thread_count << endl;
    if ((avcodec_open2(codec_ctx_video, videoCodec, NULL)) < 0) {
        cout << "cannot open specified audio codec" << endl;
    }

    if (init_sdl()) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    //分配解码后的数据存储位置
    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();

    av_dump_format(fmt_ctx, 0, source_url, 0);
    SDL_Event windowEvent;

    // 读帧
    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (SDL_QUIT == windowEvent.type) {
                cout << "SDL quit!!" << endl;
                break;
            }
        }

        int result = av_read_frame(fmt_ctx, avPacket);
        if (result < 0) {
            cout << "end of file" << endl;
//            avcodec_send_packet(codec_ctx_video, NULL); // TODO有一个最后几帧收不到的问题需要这段代码调用解决
            break;
        }

        if (avPacket->stream_index != index_video_stream) { // 目前只显示视频数据
            av_packet_unref(avPacket); // 注意清理，容易造成内存泄漏
            continue;
        }
        // 发送到解码器线程解码， avPacket会被复制一份，所以avPacket可以直接清理掉
        result = avcodec_send_packet(codec_ctx_video, avPacket);
        av_packet_unref(avPacket); // 注意清理，容易造成内存泄漏
        if (result != 0) {
            cout << "av_packet_unref failed" << endl;
            continue;
        }
        while (true) { // 接收解码后的数据, 解码是在后台线程，packet 和 frame并不是一一对应，多次收帧防止漏掉。
            result = avcodec_receive_frame(codec_ctx_video, avFrame);
            if (result != 0) { // 收帧失败，说明现在还没有解码好的帧数据，退出收帧动作。
                break;
            }
            // 像素格式刚好是YUV420P的，不用做像素格式转换
            cout << "avFrame pts : " << avFrame->pts << " color format:" << avFrame->format << " avFrame->linesize[0]:"
                 << avFrame->linesize[0] << " avFrame->linesize[1]:" << avFrame->linesize[1] << " avFrame->linesize[2]:"
                 << avFrame->linesize[2] << endl;
//            result = SDL_UpdateTexture(texture, NULL, avFrame->data[0], avFrame->linesize[0]); // 使用该函数会造成crash
            result = SDL_UpdateYUVTexture(texture, NULL, avFrame->data[0], avFrame->linesize[0],
                                          avFrame->data[1], avFrame->linesize[1], avFrame->data[2],
                                          avFrame->linesize[2]);
            if (result != 0) {
                cout << "SDL_UpdateTexture failed" << endl;
                continue;
            }
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_Delay(40); // 防止显示过快，如果要实现倍速播放，只需要调整delay时间就可以了。
        }
    }
    free_sdl();
    return 0;
}

int init_sdl() {
    cout << "init sdl, mWidth:" << mWidth << ",mHeight:" << mHeight << endl;
    // 初始化sdl
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { // 目前只需要播放视频
        cout << "SDL could not initialized with error: " << SDL_GetError() << endl;
        return -1;
    }
    // 创建窗体
    sdl_window = SDL_CreateWindow("FFmpeg+SDL播放视频", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  mWidth, mHeight, SDL_WINDOW_ALLOW_HIGHDPI);
    if (sdl_window == NULL) {
        cout << "SDL could not create window with error: " << SDL_GetError() << endl;
        return -1;
    }
    // 从窗体创建渲染器
    renderer = SDL_CreateRenderer(sdl_window, -1, 0);
    if (!renderer) {
        cout << "create renderer failed" << endl;
        return -1;
    }
    // 创建渲染器纹理，我的视频编码格式是SDL_PIXELFORMAT_IYUV，可以从FFmpeg探测到的实际结果设置颜色格式
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, mWidth, mHeight);
    if (!texture) {
        cout << "create texture failed;" << endl;
        return -1;
    }
    return 0;
}

int free_sdl() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
