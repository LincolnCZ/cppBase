#ifndef _VIDEO_DECODER_H_
#define _VIDEO_DECODER_H_

#include <vector>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class CVideoDecoder {
public:
    CVideoDecoder(std::string taskId);
    virtual ~CVideoDecoder();

    int init(AVCodecParameters *codecpar);
    void release();
    int decodeFrame(AVPacket *pkt, AVFrame *outFrame);
    int flushDecoder(AVFrame *outFrame);

private:
    std::string m_taskId;
    AVCodecContext *m_pCodecCtx;
    uint32_t m_decoderInFrames;
    uint32_t m_decoderOutFrames;
};

#endif