#include "logger.h"
#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

static void ffmpeg_log(void *ptr, int level, const char *fmt, va_list va) {
    int l = Debug;
    if (level == AV_LOG_INFO)
        l = Info;
    else if (level == AV_LOG_WARNING)
        l = Warn;
    else if (level < AV_LOG_WARNING)
        l = Error;

    if (l > getLogLevel())
        return;
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, va);
    LOG(l, "[ffmpeg] %s", buf);
}

static void doInitFFmpeg() {
    av_log_set_callback(ffmpeg_log);
    av_register_all();
}

void initFFmpeg() {
    static int inited = 0;
    static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
    if (inited == 0) {
        pthread_mutex_lock(&mut);
        if (inited == 0) {
            doInitFFmpeg();
            inited = 1;
        }
        pthread_mutex_unlock(&mut);
    }
}

AVStream *find_stream(AVFormatContext *fmt_ctx, enum AVMediaType type) {
    AVStream *stream = NULL;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == type) {
            stream = fmt_ctx->streams[i];
            break;
        }
    }
    return stream;
}