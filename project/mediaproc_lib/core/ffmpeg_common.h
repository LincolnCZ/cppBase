#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

extern void initFFmpeg();
extern AVStream *find_stream(AVFormatContext *fmt_ctx, enum AVMediaType type);

// compare ts in timebase and time
// -1 if ts < time, 0 if ts == time, 1 if ts > time
inline int compare_timestamp(int64_t ts, AVRational timebase, int time) {
    int64_t diff = ts * timebase.num - (int64_t)time * timebase.den;
    if(diff < 0)
        return -1;
    else if(diff > 0)
        return 1;
    else
        return 0;
}

#ifdef __cplusplus
}
#endif