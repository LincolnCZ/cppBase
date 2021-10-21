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
#include <ctime>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
};

using namespace std;

struct SwrContext *g_au_convert_ctx;
int g_audio_out_buffer_size;
SDL_Event g_sdl_event;

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//Output PCM
#define OUTPUT_PCM 1
//Use SDL
#define USE_SDL 1

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
                swr_convert(g_au_convert_ctx, &audio_buf, MAX_AUDIO_FRAME_SIZE, (const uint8_t **) frame->data,
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

//    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
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
    wanted_spec.callback = audio_callback;
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
    int64_t in_channel_layout;

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

    // audio_st = pFormatCtx->streams[index].
    packet_queue_init(&audioq);

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
    g_audio_out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
    cout << "g_audio_out_buffer_size :" << g_audio_out_buffer_size << endl;

    //FIX:Some Codec's Context Information is missing
    in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);

    //Swr
    g_au_convert_ctx = swr_alloc();
    g_au_convert_ctx = swr_alloc_set_opts(g_au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                          in_channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
    swr_init(g_au_convert_ctx);

    // init sdl
    if (init_sdl(out_sample_rate, aCodecCtx->channels, out_nb_samples, aCodecCtx)) {
        cout << "init sdl failed" << endl;
        exit(1);
    }

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audioStreamIndex) {
            packet_queue_put(&audioq, packet);
        } else {
            // Free the packet that was allocated by av_read_frame.
            av_packet_unref(packet);
        }

//        SDL_PollEvent(&g_sdl_event);
//        switch (g_sdl_event.type) {
//            case SDL_QUIT:
//                quit = 1;
//                SDL_Quit();
//                exit(0);
//                break;
//            default:
//                break;
//        }
    }

    getchar();

    swr_free(&g_au_convert_ctx);

#if USE_SDL
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
#endif

#if OUTPUT_PCM
    fclose(pFile);
#endif
    avcodec_close(aCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
