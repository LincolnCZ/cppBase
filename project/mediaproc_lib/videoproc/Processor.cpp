#include <memory>
#include "core/logger.h"
#include "core/utils.h"
#include "core/ffmpeg_common.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "Processor.h"
#include "VideoDecoder.h"
#include "ImageEncoder.h"

static int coverPts2sec(int64_t ts, AVRational timebase) {
    if (timebase.den <= 0) {
        return 0;
    }

    return ts * timebase.num / timebase.den;
}

bool cutFrames(const std::string &dataId, const std::string &input, std::vector<CutFrameInfo> &output,
               int frameInterval, int timeInterval, int maxFrames, double &duration) {
    initFFmpeg();
    output.clear();

    // 打开输入文件信息
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    if (avformat_open_input(&fmt_ctx, input.c_str(), NULL, NULL) != 0) {
        avformat_free_context(fmt_ctx);
        FUNLOG(Warn, "%s open avformat fail", dataId.c_str());
        return false;
    }
    // avformat_close_input 会自动调用 avformat_free_context
    auto _defer_fmt = utility::defer([&fmt_ctx]() { avformat_close_input(&fmt_ctx); });

    // 打开查找音视频流信息
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        FUNLOG(Warn, "%s find stream info fail", dataId.c_str());
        return false;
    }

    AVStream *stream_in = find_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO);
    if (stream_in == NULL) {
        FUNLOG(Warn, "%s find video stream fail", dataId.c_str());
        return false;
    }

    // av_dump_format(fmt_ctx, 0, "input", 0);
    duration = av_q2d(av_make_q(stream_in->time_base.num * stream_in->duration, stream_in->time_base.den));
    FUNLOG(Info, "%s stream in time_base %d/%d duration %lf",
           dataId.c_str(), stream_in->time_base.num, stream_in->time_base.den, duration);

    CVideoDecoder decoder(dataId);
    if (0 != decoder.init(stream_in->codecpar)) {
        FUNLOG(Error, "%s init decoder fail", dataId.c_str());
        return false;
    }

    AVFrame *outFrame = av_frame_alloc();
    if (outFrame == NULL) {
        FUNLOG(Error, "%s av_frame_alloc fail", dataId.c_str());
        return false;
    }

    auto _defer_frame = utility::defer([&outFrame]() { av_frame_free(&outFrame); });

    CImageEncoder *imageEncoder = new CImageEncoder(dataId, CImageEncoder::JPEG);
    auto _defer_encoder = utility::defer([imageEncoder]() { delete imageEncoder; });

    bool result = true;
    int64_t base_pts = -1;
    int64_t last_pts = 0;

    int64_t decode_count = 0;
    int64_t encode_count = 0;
    int64_t last_encode_index = 0;

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    while (av_read_frame(fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == stream_in->index) {
            //decode
            int ret = decoder.decodeFrame(&packet, outFrame);
            if (ret <= 0) {
                if (ret < 0)
                    FUNLOG(Error, "%s decode frame fail, ret=%d", dataId.c_str(), ret);
                av_free_packet(&packet);
                continue;
            }

            decode_count++;
            if (base_pts < 0) {
                base_pts = packet.pts;
            }

            bool shouldEncode = false;
            if (frameInterval > 0) {
                if (encode_count == 0 || decode_count - last_encode_index >= frameInterval) {
                    shouldEncode = true;
                    FUNLOG(Info,
                           "%s should cut one frame now! frameInterval=%d, decode_count=%ld, last_encode_index=%ld",
                           dataId.c_str(), frameInterval, decode_count, last_encode_index);
                }
            } else if (timeInterval > 0) {
                int64_t diff_pts = packet.pts - last_pts;
                if (encode_count == 0 || compare_timestamp(diff_pts, stream_in->time_base, timeInterval) >= 0) {
                    shouldEncode = true;
                    FUNLOG(Info,
                           "%s should cut one frame now! diff_pts*(timebase.num/timebase.den)=%ld*(%d/%d)  >= timeInterval=%d",
                           dataId.c_str(), diff_pts, stream_in->time_base.num, stream_in->time_base.den, timeInterval);
                }
            }

            if (shouldEncode) {
                std::string picOutput;
                if (imageEncoder->encode(outFrame, picOutput) == 0) {
                    encode_count++;
                    last_pts = packet.pts;
                    last_encode_index = decode_count;

                    CutFrameInfo frameInfo;
                    frameInfo.content = picOutput;
                    frameInfo.timeAt = coverPts2sec(packet.pts - base_pts, stream_in->time_base);
                    output.push_back(frameInfo);
                } else {
                    FUNLOG(Error, "%s encode jpg fail!", dataId.c_str());
                }
            }
        }
        av_free_packet(&packet);

        if (encode_count >= maxFrames) {
            break;
        }
    }

    //decoder.flushDecoder(outFrame);
    decoder.release();
    return result;
}
