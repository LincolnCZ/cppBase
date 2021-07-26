#undef __CLASS__
#define __CLASS__ "CImageEncoder"

#include <string>
#include "core/logger.h"
#include "ImageEncoder.h"

CImageEncoder::CImageEncoder(std::string taskId, ImageType imageType)
{
    m_taskId = taskId;
    m_imageType = imageType;
    m_codecCtx = NULL;
    switch (imageType)
    {
        case JPEG:
        default:
            m_codecId = AV_CODEC_ID_MJPEG;
            break;
    }
}

CImageEncoder::~CImageEncoder()
{
    release();
}

int CImageEncoder::encode(AVFrame *pFrame, std::string &output)
{
    if (m_codecCtx == NULL && !init(pFrame))
        return -1;

    // source frame resolution changed, reinit the codec context
    if (m_codecCtx->height != pFrame->height || m_codecCtx->width != pFrame->width)
    {
        FUNLOG(Info, "%s resolution change from %dx%d to %dx%d, reinit image encoder.", m_taskId.c_str(),
            m_codecCtx->width, m_codecCtx->height,
            pFrame->width, pFrame->height);

        release();
        if (!init(pFrame))
            return -1;
    }

    int gotFrame = 0;

	AVPacket pkt;
	av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
	int ret = avcodec_encode_video2(m_codecCtx, &pkt, pFrame, &gotFrame);
	if (ret < 0 || gotFrame == 0) {
		av_packet_unref(&pkt);
		FUNLOG(Error, "%s encode image failed, ret=%d, gotFrame=%d", m_taskId.c_str(), ret, gotFrame);
		return -1;
	}
    output.assign((const char*)pkt.data, pkt.size);
	av_packet_unref(&pkt);

    FUNLOG(Info, "%s encode image success. size=%zu", m_taskId.c_str(), output.size());
    return 0;
}

bool CImageEncoder::init(AVFrame *pFrame)
{
    FUNLOG(Info, "%s init with width=%d, height=%d.", m_taskId.c_str(), pFrame->width, pFrame->height);

    AVCodec *codec = avcodec_find_encoder(m_codecId);
    if (!codec)
    {
        FUNLOG(Error, "%s AVCodec not found with codecId=%d.", m_taskId.c_str(), m_codecId);
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx)
    {
        FUNLOG(Error, "%s can not allocate avcodec context.", m_taskId.c_str());
        return false;
    }

    m_codecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    m_codecCtx->height = pFrame->height;
    m_codecCtx->width = pFrame->width;
    m_codecCtx->time_base.num = 1;
    m_codecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    m_codecCtx->global_quality = 50 * FF_QP2LAMBDA;

    int ret = 0;
    if ((ret = avcodec_open2(m_codecCtx, codec, NULL)) < 0)
    {
        FUNLOG(Error, "%s can not open avcodec, ret=%d", m_taskId.c_str(), ret);
        return false;
    }

    return true;
}

void CImageEncoder::release()
{
    if (m_codecCtx != NULL)
    {
        avcodec_close(m_codecCtx);
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = NULL;
    }

    FUNLOG(Info, "%s released", m_taskId.c_str());
}

int CImageEncoder::getWidth()
{
    if (m_codecCtx == NULL)
        return 0;

    return m_codecCtx->width;
}

int CImageEncoder::getHeight()
{
    if (m_codecCtx == NULL)
        return 0;

    return m_codecCtx->height;
}
