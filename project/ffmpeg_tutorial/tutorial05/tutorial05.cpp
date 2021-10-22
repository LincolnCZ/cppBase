// tutorial04.c
// A pedagogical video player that will stream through every video frame as fast as it can,
// and play audio (out of sync).

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>

#include <SDL.h>
#include <SDL_thread.h>
}

#include <cstdio>
#include <cmath>
#include <iostream>

using namespace std;

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define FF_ALLOC_EVENT (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)

#define VIDEO_PICTURE_QUEUE_SIZE 1

// SDL
SDL_Window *g_sdl_window;
SDL_Renderer *g_sdl_renderer;

uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;

typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;


typedef struct VideoPicture {
    SDL_Texture *texture; // todo,sdl相关的需要释放资源
    int width, height; // Source height & width.
    int allocated;
    double pts;

    AVFrame *pFrameYUV; // todo，目前队列中只有一个元素，所以只保存一个指针就够了，后续需要更改
} VideoPicture;

typedef struct VideoState {
    AVFormatContext *pFormatCtx;
    int videoStream, audioStream;

    // ----音频处理相关------------------
    double audio_clock;
    AVStream *audio_st;
    PacketQueue audioq;
    uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size;
    unsigned int audio_buf_index;
    struct SwrContext *audio_convert_ctx;
    int audio_out_buffer_size;

    AVFrame audio_frame;
    AVPacket audio_pkt;
    uint8_t *audio_pkt_data;
    int audio_pkt_size;


    // ----视频处理相关------------------
    int audio_hw_buf_size;
    double frame_timer;
    double frame_last_pts;
    double frame_last_delay;
    double video_clock; // pts of last decoded frame / predicted pts of next decoded frame.
    AVStream *video_st;
    PacketQueue videoq;
    // 视频数据缓冲区 pictq 来存储解码后的视频帧
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex; // 读写索引
    SDL_mutex *pictq_mutex;
    SDL_cond *pictq_cond;

    SDL_Thread *parse_tid;
    SDL_Thread *video_tid;

    char filename[1024];
    int quit;

    AVIOContext *io_context;
    struct SwsContext *sws_ctx;
} VideoState;

//SDL_Surface *screen;
SDL_mutex *screen_mutex;

// Since we only have one decoding thread, the Big Struct can be global in case we need it.
VideoState *global_video_state;

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

        if (global_video_state->quit) {
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

int audio_decode_frame(VideoState *is, double *pts_ptr, uint8_t *audio_buf) {
    int len1, data_size = 0, n;
    AVPacket *pkt = &is->audio_pkt;
    double pts;

    for (;;) {
        while (is->audio_pkt_size > 0) {
            int got_frame;
            len1 = avcodec_decode_audio4(is->audio_st->codec, &is->audio_frame, &got_frame, pkt);
            if (len1 < 0) {
                // If error, skip frame.
                is->audio_pkt_size = 0;
                break;
            }
            if (got_frame) {
                swr_convert(is->audio_convert_ctx, &audio_buf, MAX_AUDIO_FRAME_SIZE,
                            (const uint8_t **) is->audio_frame.data, is->audio_frame.nb_samples);
            }
            is->audio_pkt_data += len1;
            is->audio_pkt_size -= len1;

            pts = is->audio_clock;
            *pts_ptr = pts;
            n = 2 * is->audio_st->codec->channels;
            is->audio_clock += (double) data_size / (double) (n * is->audio_st->codec->sample_rate);

            // We have data, return it and come back for more later.
            return is->audio_out_buffer_size;
        }
        if (pkt->data) {
            av_packet_unref(pkt);
        }

        if (is->quit) {
            return -1;
        }
        // Next packet.
        if (packet_queue_get(&is->audioq, pkt, 1) < 0) {
            return -1;
        }
        is->audio_pkt_data = pkt->data;
        is->audio_pkt_size = pkt->size;
        // If update, update the audio clock w/pts.
        if (pkt->pts != AV_NOPTS_VALUE) {
            is->audio_clock = av_q2d(is->audio_st->time_base) * pkt->pts;
        }
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
    VideoState *is = (VideoState *) userdata;
    int len1, audio_size;
    double pts;

    SDL_memset(stream, 0, len);

    while (len > 0) {
        if (is->audio_buf_index >= is->audio_buf_size) {
            // We have already sent all our data; get more.
            audio_size = audio_decode_frame(is, &pts, is->audio_buf);
            if (audio_size < 0) {
                // If error, output silence.
                is->audio_buf_size = 1024;
                memset(is->audio_buf, 0, is->audio_buf_size);
            } else {
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }

        SDL_MixAudio(stream, (uint8_t *) is->audio_buf + is->audio_buf_index, len1, SDL_MIX_MAXVOLUME);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = opaque;
    SDL_PushEvent(&event);
    return 0; // 0 means stop timer.
}

// Schedule a video refresh in 'delay' ms.
static void schedule_refresh(VideoState *is, int delay) {
    SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

void video_display(VideoState *is) {
    SDL_Rect rect;
    VideoPicture *vp;
    float aspect_ratio;
    int w, h, x, y;

    vp = &is->pictq[is->pictq_rindex];
    if (vp->texture) {
        if (is->video_st->codec->sample_aspect_ratio.num == 0) {
            aspect_ratio = 0;
        } else {
            aspect_ratio = av_q2d(is->video_st->codec->sample_aspect_ratio) * is->video_st->codec->width /
                           is->video_st->codec->height;
        }
        if (aspect_ratio <= 0.0) {
            aspect_ratio = (float) is->video_st->codec->width / (float) is->video_st->codec->height;
        }

        int screen_w;
        SDL_GetWindowSize(g_sdl_window, &screen_w, &h);

//        h = screen->h;
        w = ((int) rint(h * aspect_ratio)) & -3;
        if (w > screen_w) {
            w = screen_w;
            h = ((int) rint(w / aspect_ratio)) & -3;
        }
        x = (screen_w - w) / 2;
        y = (h - h) / 2;

        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;

        SDL_LockMutex(screen_mutex);

        SDL_UpdateTexture(vp->texture, NULL, vp->pFrameYUV->data[0], vp->pFrameYUV->linesize[0]);
        SDL_RenderClear(g_sdl_renderer);
        SDL_RenderCopy(g_sdl_renderer, vp->texture, NULL, NULL);
        SDL_RenderPresent(g_sdl_renderer);

        SDL_UnlockMutex(screen_mutex);
    }
}

double get_audio_clock(VideoState *is) {
    double pts;
    int hw_buf_size, bytes_per_sec, n;

    pts = is->audio_clock; // Maintained in the audio thread.
    hw_buf_size = is->audio_buf_size - is->audio_buf_index;
    bytes_per_sec = 0;
    n = is->audio_st->codec->channels * 2;
    if (is->audio_st) {
        bytes_per_sec = is->audio_st->codec->sample_rate * n;
    }
    if (bytes_per_sec) {
        pts -= (double) hw_buf_size / bytes_per_sec;
    }
    return pts;
}

void video_refresh_timer(void *userdata) {
    VideoState *is = (VideoState *) userdata;
    VideoPicture *vp;
    double actual_delay, delay, sync_threshold, ref_clock, diff;

    if (is->video_st) {
        if (is->pictq_size == 0) {
            schedule_refresh(is, 1);
        } else {
            vp = &is->pictq[is->pictq_rindex];

            delay = vp->pts - is->frame_last_pts; // The pts from last time.
            if (delay <= 0 || delay >= 1.0) {
                // If incorrect delay, use previous one.
                delay = is->frame_last_delay;
            }
            // Save for next time.
            is->frame_last_delay = delay;
            is->frame_last_pts = vp->pts;

            // Update delay to sync to audio.
            ref_clock = get_audio_clock(is);
            diff = vp->pts - ref_clock;

            // Skip or repeat the frame. Take delay into account FFPlay still doesn't "know if this is the best guess."
            sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
            if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
                if (diff <= -sync_threshold) {
                    delay = 0;
                } else if (diff >= sync_threshold) {
                    delay = 2 * delay;
                }
            }
            is->frame_timer += delay;
            // Computer the REAL delay.
            actual_delay = is->frame_timer - (av_gettime() / 1000000.0);
            if (actual_delay < 0.010) {
                // Really it should skip the picture instead.
                actual_delay = 0.010;
            }
            schedule_refresh(is, (int) (actual_delay * 1000 + 0.5));
            // Show the picture!
            video_display(is);

            // Update queue for next picture!
            if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                is->pictq_rindex = 0;
            }
            SDL_LockMutex(is->pictq_mutex);
            is->pictq_size--;
            SDL_CondSignal(is->pictq_cond);
            SDL_UnlockMutex(is->pictq_mutex);
        }
    } else {
        schedule_refresh(is, 100);
    }
}

void alloc_picture(void *userdata) {

    VideoState *is = (VideoState *) userdata;
    VideoPicture *vp;

    vp = &is->pictq[is->pictq_windex];
    if (vp->texture) {
        // We already have one make another, bigger/smaller.
//        SDL_FreeYUVOverlay(vp->bmp);
        SDL_DestroyTexture(vp->texture);
    }
    // Allocate a place to put our YUV image on that screen.
    SDL_LockMutex(screen_mutex);
    vp->texture = SDL_CreateTexture(g_sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                    is->video_st->codec->width, is->video_st->codec->height);

    vp->pFrameYUV = av_frame_alloc();
    unsigned char *out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, is->video_st->codec->width, is->video_st->codec->height, 1));
    av_image_fill_arrays(vp->pFrameYUV->data, vp->pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, is->video_st->codec->width, is->video_st->codec->height, 1);

    SDL_UnlockMutex(screen_mutex);
    vp->width = is->video_st->codec->width;
    vp->height = is->video_st->codec->height;

    SDL_LockMutex(is->pictq_mutex);
    vp->allocated = 1;
    SDL_CondSignal(is->pictq_cond);
    SDL_UnlockMutex(is->pictq_mutex);
}

int queue_picture(VideoState *is, AVFrame *pFrame, double pts) {
    VideoPicture *vp;
    AVFrame pict;

    // Wait until we have space for a new pic.
    SDL_LockMutex(is->pictq_mutex);
    while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->quit) {
        SDL_CondWait(is->pictq_cond, is->pictq_mutex);
    }
    SDL_UnlockMutex(is->pictq_mutex);

    if (is->quit) {
        return -1;
    }

    // windex is set to 0 initially.
    vp = &is->pictq[is->pictq_windex];

    // Allocate or resize the buffer!
    if (!vp->texture || vp->width != is->video_st->codec->width || vp->height != is->video_st->codec->height) {
        SDL_Event event;

        vp->allocated = 0;
        // We have to do it in the main thread.
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);

        // Wait until we have a picture allocated.
        SDL_LockMutex(is->pictq_mutex);
        while (!vp->allocated && !is->quit) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
        SDL_UnlockMutex(is->pictq_mutex);
        if (is->quit) {
            return -1;
        }
    }

    // We have a place to put our picture on the queue.
    if (vp->texture) {
        // Convert the image into YUV format that SDL uses.
        sws_scale(is->sws_ctx, (uint8_t const *const *) pFrame->data, pFrame->linesize, 0, is->video_st->codec->height,
                  vp->pFrameYUV->data, vp->pFrameYUV->linesize);

        vp->pts = pts;

        // Now we inform our display thread that we have a pic ready.
        if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
            is->pictq_windex = 0;
        }
        SDL_LockMutex(is->pictq_mutex);
        is->pictq_size++;
        SDL_UnlockMutex(is->pictq_mutex);
    }
    return 0;
}

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {
    double frame_delay;

    if (pts != 0) {
        // If we have pts, set video clock to it.
        is->video_clock = pts;
    } else {
        // If we aren't given a pts, set it to the clock.
        pts = is->video_clock;
    }
    // Update the video clock.
    frame_delay = av_q2d(is->video_st->codec->time_base);
    // If we are repeating a frame, adjust clock accordingly.
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}

int video_thread(void *arg) {
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    int frameFinished;
    AVFrame *pFrame;
    double pts;

    pFrame = av_frame_alloc();
    for (;;) {
        if (packet_queue_get(&is->videoq, packet, 1) < 0) {
            // Means we quit getting packets.
            break;
        }

        pts = 0;
        // Save global pts to be stored in pFrame in first call.
        global_video_pkt_pts = packet->pts;
        // Decode video frame.
        avcodec_decode_video2(is->video_st->codec, pFrame, &frameFinished, packet);
        if (packet->dts == AV_NOPTS_VALUE && pFrame->opaque && *(uint64_t *) pFrame->opaque != AV_NOPTS_VALUE) {
            pts = *(uint64_t *) pFrame->opaque;
        } else if (packet->dts != AV_NOPTS_VALUE) {
            pts = packet->dts;
        } else {
            pts = 0;
        }
        pts *= av_q2d(is->video_st->time_base);

        // Did we get a video frame?
        if (frameFinished) {
            pts = synchronize_video(is, pFrame, pts);
            if (queue_picture(is, pFrame, pts) < 0) {
                break;
            }
        }
        av_packet_unref(packet);
    }

    av_free(pFrame);
    return 0;
}

int stream_component_open(VideoState *is, int stream_index) {

    AVFormatContext *pFormatCtx = is->pFormatCtx;
    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;
    AVDictionary *optionsDict = NULL;
    SDL_AudioSpec wanted_spec, spec;

    if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
        return -1;
    }

    // Get a pointer to the codec context for the video stream.
    codecCtx = pFormatCtx->streams[stream_index]->codec;

    if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {

        //---------------------------
        //Out Audio Param
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        //nb_samples: AAC-1024 MP3-1152
        int out_nb_samples = codecCtx->frame_size;
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
        int out_sample_rate = codecCtx->sample_rate;
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

        //Out Buffer Size
        is->audio_out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
        cout << "audio_out_buffer_size :" << is->audio_out_buffer_size << endl;

//    //FIX:Some Codec's Context Information is missing
        int64_t in_channel_layout = av_get_default_channel_layout(codecCtx->channels);

        //Swr
        is->audio_convert_ctx = swr_alloc();
        is->audio_convert_ctx = swr_alloc_set_opts(is->audio_convert_ctx, out_channel_layout, out_sample_fmt,
                                                   out_sample_rate, in_channel_layout, codecCtx->sample_fmt,
                                                   codecCtx->sample_rate, 0, NULL);
        swr_init(is->audio_convert_ctx);
        //------------------------------

        // Set audio settings from codec info.
        wanted_spec.freq = out_sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = out_channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = out_nb_samples;
        wanted_spec.callback = audio_callback;
        wanted_spec.userdata = is;

        if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
            fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
            return -1;
        }
    }
    codec = avcodec_find_decoder(codecCtx->codec_id);
    if (!codec || (avcodec_open2(codecCtx, codec, &optionsDict) < 0)) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    switch (codecCtx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            is->audioStream = stream_index;
            is->audio_st = pFormatCtx->streams[stream_index];
            is->audio_buf_size = 0;
            is->audio_buf_index = 0;
            memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
            packet_queue_init(&is->audioq);
            SDL_PauseAudio(0);
            break;
        case AVMEDIA_TYPE_VIDEO:
            is->videoStream = stream_index;
            is->video_st = pFormatCtx->streams[stream_index];

            is->frame_timer = (double) av_gettime() / 1000000.0;
            is->frame_last_delay = 40e-3;

            packet_queue_init(&is->videoq);
            is->video_tid = SDL_CreateThread(video_thread, "video thread", is);
            is->sws_ctx = sws_getContext(is->video_st->codec->width, is->video_st->codec->height,
                                         is->video_st->codec->pix_fmt, is->video_st->codec->width,
                                         is->video_st->codec->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
                                         NULL, NULL, NULL);
            break;
        default:
            break;
    }
    return 0;
}

int decode_interrupt_cb(void *opaque) {
    return (global_video_state && global_video_state->quit);
}

int decode_thread(void *arg) {

    VideoState *is = (VideoState *) arg;
    AVFormatContext *pFormatCtx = NULL;
    AVPacket pkt1, *packet = &pkt1;

    int video_index = -1;
    int audio_index = -1;
    int i;

    AVDictionary *io_dict = NULL;
    AVIOInterruptCB callback;

    is->videoStream = -1;
    is->audioStream = -1;

    global_video_state = is;
    // will interrupt blocking functions if we quit!.
    callback.callback = decode_interrupt_cb;
    callback.opaque = is;
    if (avio_open2(&is->io_context, is->filename, 0, &callback, &io_dict)) {
        fprintf(stderr, "Unable to open I/O for %s\n", is->filename);
        return -1;
    }

    // Open video file.
    if (avformat_open_input(&pFormatCtx, is->filename, NULL, NULL) != 0) {
        return -1; // Couldn't open file.
    }

    is->pFormatCtx = pFormatCtx;

    // Retrieve stream information.
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -1; // Couldn't find stream information.
    }

    // Dump information about file onto standard error.
    av_dump_format(pFormatCtx, 0, is->filename, 0);

    // Find the first video stream.
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video_index < 0) {
            video_index = i;
        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio_index < 0) {
            audio_index = i;
        }
    }
    if (audio_index >= 0) {
        stream_component_open(is, audio_index);
    }
    if (video_index >= 0) {
        stream_component_open(is, video_index);
    }

    if (is->videoStream < 0 || is->audioStream < 0) {
        fprintf(stderr, "%s: could not open codecs\n", is->filename);
        goto fail;
    }

    // Main decode loop.
    for (;;) {
        if (is->quit) {
            break;
        }
        // Seek stuff goes here.
        if (is->audioq.size > MAX_AUDIOQ_SIZE || is->videoq.size > MAX_VIDEOQ_SIZE) {
            SDL_Delay(10);
            continue;
        }
        if (av_read_frame(is->pFormatCtx, packet) < 0) {
            if (is->pFormatCtx->pb->error == 0) {
                SDL_Delay(100); // No error; wait for user input.
                continue;
            } else {
                break;
            }
        }
        // Is this a packet from the video stream?
        if (packet->stream_index == is->videoStream) {
            packet_queue_put(&is->videoq, packet);
        } else if (packet->stream_index == is->audioStream) {
            packet_queue_put(&is->audioq, packet);
        } else {
            av_packet_unref(packet);
        }
    }
    // All done - wait for it.
    while (!is->quit) {
        SDL_Delay(100);
    }

    fail:
    if (1) {
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    SDL_Event event;

    VideoState *is;

    is = (VideoState *) av_mallocz(sizeof(VideoState));

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

    // 创建窗体
    g_sdl_window = SDL_CreateWindow("FFmpeg+SDL播放视频", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    640, 480, SDL_WINDOW_ALLOW_HIGHDPI);
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

    screen_mutex = SDL_CreateMutex();

    av_strlcpy(is->filename, argv[1], sizeof(is->filename));

    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond = SDL_CreateCond();

    schedule_refresh(is, 40);

    is->parse_tid = SDL_CreateThread(decode_thread, "parse thread", is);
    if (!is->parse_tid) {
        av_free(is);
        return -1;
    }
    for (;;) {

        SDL_WaitEvent(&event);
        switch (event.type) {
            case FF_QUIT_EVENT:
            case SDL_QUIT:
                is->quit = 1;
                // If the video has finished playing, then both the picture and audio queues are waiting for more data.  Make them stop waiting and terminate normally..
                SDL_CondSignal(is->audioq.cond);
                SDL_CondSignal(is->videoq.cond);
                SDL_Quit();
                return 0;
                break;
            case FF_ALLOC_EVENT:
                alloc_picture(event.user.data1);
                break;
            case FF_REFRESH_EVENT:
                video_refresh_timer(event.user.data1);
                break;
            default:
                break;
        }
    }
    return 0;
}
