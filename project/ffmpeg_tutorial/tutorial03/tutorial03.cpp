// tutorial03.c
// A pedagogical video player that will stream through every video frame as fast as it can
// and play audio (out of sync).

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

#include <SDL2/SDL.h>
#include <SDL_thread.h>
}

#include <cstdio>
#include <iostream>

using namespace std;

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

struct SwrContext *g_audio_convert_ctx;
int g_audio_out_buffer_size;

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

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf) {
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame *frame = NULL;
    if (!frame) frame = av_frame_alloc();

    int len1;

    for (;;) {
        while (audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, frame, &got_frame, &pkt);
            if (len1 < 0) {
                // if error, skip frame.
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            if (got_frame) {
                swr_convert(g_audio_convert_ctx, &audio_buf, MAX_AUDIO_FRAME_SIZE, (const uint8_t **) frame->data,
                            frame->nb_samples);
            }
            return g_audio_out_buffer_size;
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

    SDL_memset(stream, 0, len);

    static uint8_t audio_buf[MAX_AUDIO_FRAME_SIZE * 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while (len > 0) {
        if (audio_buf_index >= audio_buf_size) {
            // We have already sent all our data; get more.
            audio_size = audio_decode_frame(aCodecCtx, audio_buf);
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

        SDL_MixAudio(stream, (uint8_t *) audio_buf + audio_buf_index, len1, SDL_MIX_MAXVOLUME);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

//SDL
SDL_Renderer *g_sdl_renderer;
SDL_Texture *g_sdl_texture;
SDL_Window *g_sdl_window;
SDL_Event g_sdl_event;

int init_sdl(int width, int height, int out_sample_rate, int out_channels, int out_nb_samples,
             AVCodecContext *aCodecCtx) {
    cout << "init sdl, width:" << width << ",height:" << height << endl;
    //Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        cout << "Could not initialize SDL - :" << SDL_GetError() << endl;
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
    g_sdl_texture = SDL_CreateTexture(g_sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width,
                                      height);
    if (!g_sdl_texture) {
        cout << "create g_sdl_texture failed;" << endl;
        return -1;
    }

    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = aCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
        cout << "can't open audio." << endl;
        return -1;
    }

    //Play
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
    int i, videoStreamIndex, audioStreamIndex;
    AVCodecContext *vCodecCtx = NULL;
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
        cout << "Usage: test <file>" << endl;
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
    audioStreamIndex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIndex < 0) {
            videoStreamIndex = i;
        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStreamIndex < 0) {
            audioStreamIndex = i;
        }
    }
    if (videoStreamIndex == -1) {
        return -1; // Didn't find a video stream.
    }
    if (audioStreamIndex == -1) {
        return -1;
    }

    // Get a pointer to the codec context for the video stream.
    vCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;

    // Find the decoder for the video stream.
    pCodec = avcodec_find_decoder(vCodecCtx->codec_id);
    if (pCodec == NULL) {
        cout << "Unsupported codec!" << endl;
        return -1; // Codec not found.
    }
    // Open codec.
    if (avcodec_open2(vCodecCtx, pCodec, &videoOptionsDict) < 0)
        return -1; // Could not open codec.

    // Allocate video frame.
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    unsigned char *out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, vCodecCtx->width, vCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, vCodecCtx->width, vCodecCtx->height, 1);

    // audio_st = pFormatCtx->streams[index].
    packet_queue_init(&audioq);

    // audio codec
    aCodecCtx = pFormatCtx->streams[audioStreamIndex]->codec;
    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (!aCodec) {
        cout << "Unsupported codec!" << endl;
        return -1;
    }

    avcodec_open2(aCodecCtx, aCodec, &audioOptionsDict);

    //Out Audio Param
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    //nb_samples: AAC-1024 MP3-1152
    int out_nb_samples = aCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = aCodecCtx->sample_rate;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    //Out Buffer Size
    g_audio_out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
    cout << "g_audio_out_buffer_size :" << g_audio_out_buffer_size << endl;

//    //FIX:Some Codec's Context Information is missing
    int64_t in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);

    //Swr
    g_audio_convert_ctx = swr_alloc();
    g_audio_convert_ctx = swr_alloc_set_opts(g_audio_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                             in_channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
    swr_init(g_audio_convert_ctx);

    // Allocate a place to put our YUV image on that screen.
    sws_ctx = sws_getContext(vCodecCtx->width, vCodecCtx->height, vCodecCtx->pix_fmt, vCodecCtx->width,
                             vCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    // init sdl
    if (init_sdl(vCodecCtx->width, vCodecCtx->height, out_sample_rate, aCodecCtx->channels, out_nb_samples,
                 aCodecCtx)) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    // Read frames and save first five frames to disk.
    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
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

        // Is this a packet from the video stream?
        if (packet.stream_index == videoStreamIndex) {
            // Decode video frame.
            avcodec_decode_video2(vCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if (frameFinished) {

                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data, pFrame->linesize, 0, vCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //SDL---------------------------
                SDL_UpdateTexture(g_sdl_texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(g_sdl_renderer);
                SDL_RenderCopy(g_sdl_renderer, g_sdl_texture, NULL, NULL);
                SDL_RenderPresent(g_sdl_renderer);
                //SDL End-----------------------

                av_packet_unref(&packet);
            }
        } else if (packet.stream_index == audioStreamIndex) {
            packet_queue_put(&audioq, &packet);
        } else {
            // Free the packet that was allocated by av_read_frame.
            av_packet_unref(&packet);
        }
    }

    getchar();

    swr_free(&g_audio_convert_ctx);

    // Free the YUV frame.
    av_free(pFrame);

    // Close the codec.
    avcodec_close(vCodecCtx);

    // Close the video file.
    avformat_close_input(&pFormatCtx);

    free_sdl();

    return 0;
}