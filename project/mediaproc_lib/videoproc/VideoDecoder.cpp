#undef __CLASS__
#define __CLASS__ "CVideoDecoder"

#include "VideoDecoder.h"
#include "core/logger.h"

CVideoDecoder::CVideoDecoder(std::string taskId) {
    m_pCodecCtx = NULL;
    m_taskId = taskId;
    m_decoderInFrames = 0;
    m_decoderOutFrames = 0;
}

CVideoDecoder::~CVideoDecoder() {
    release();
}

int CVideoDecoder::init(AVCodecParameters *codecpar) {
    if (m_pCodecCtx != NULL) {
        FUNLOG(Warn, "%s Video decoder already initialized", m_taskId.c_str());
        return -1;
    }

    AVCodecID avCodecId = codecpar->codec_id;
    AVCodec *pDecoder = avcodec_find_decoder(avCodecId);
    if (pDecoder == NULL) {
        FUNLOG(Error, "%s Decoder %u not found", m_taskId.c_str(), avCodecId);
        return -1;
    }

    m_pCodecCtx = avcodec_alloc_context3(pDecoder);
    if (m_pCodecCtx == NULL) {
        FUNLOG(Error, "%s Failed to allocate avcodec context", m_taskId.c_str());
        return -1;
    }
    avcodec_parameters_to_context(m_pCodecCtx, codecpar);

    m_pCodecCtx->thread_count = 1;
    if (avcodec_open2(m_pCodecCtx, pDecoder, NULL) < 0) {
        FUNLOG(Error, "%s Failed to open avcodec", m_taskId.c_str());
        avcodec_free_context(&m_pCodecCtx);
        return -1;
    }

    m_decoderInFrames = 0;
    m_decoderOutFrames = 0;
    FUNLOG(Notice, "%s Video decoder initialized", m_taskId.c_str());
    return 0;
}

void CVideoDecoder::release() {
    if (m_pCodecCtx != NULL) {
        FUNLOG(Notice, "%s Video decoder released", m_taskId.c_str());
        avcodec_free_context(&m_pCodecCtx);
        m_pCodecCtx = NULL;
    }
}

int CVideoDecoder::decodeFrame(AVPacket *pkt, AVFrame *outFrame) {
    if (m_pCodecCtx == NULL || outFrame == NULL) {
        FUNLOG(Error, "%s m_pCodecCtx == NULL || outFrame == NULL", m_taskId.c_str());
        return -1;
    }

    // wait for first IFrame
    if ((AV_PKT_FLAG_KEY != (pkt->flags & AV_PKT_FLAG_KEY)) && m_decoderInFrames == 0) {
        FUNLOG(Error, "%s not I frame first!", m_taskId.c_str());
        return -2;
    }

    int gotFrame = 0;
    m_decoderInFrames++;
    int err = avcodec_decode_video2(m_pCodecCtx, outFrame, &gotFrame, pkt);
    if (err < 0) {
        FUNLOG(Error, "%s decode video error %d", m_taskId.c_str(), err);
        return -3;
    }
    m_decoderOutFrames += gotFrame;
    return gotFrame;
}

int CVideoDecoder::flushDecoder(AVFrame *outFrame) {
    if (m_pCodecCtx == NULL || outFrame == NULL)
        return -1;

    int gotFrame = 0;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    avcodec_decode_video2(m_pCodecCtx, outFrame, &gotFrame, &pkt);
    av_packet_unref(&pkt);
    return gotFrame == 0 ? -1 : 0;
}
