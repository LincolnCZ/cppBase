#ifndef _IMAGE_ENCODER_H_
#define _IMAGE_ENCODER_H_

#include <stdint.h>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class CImageEncoder {
public:
    enum ImageType {
        JPEG
    };

    CImageEncoder(std::string taskId, ImageType ImageType);
    virtual ~CImageEncoder();

    ImageType getImageType() { return m_imageType; }
    int getWidth();
    int getHeight();

    int encode(AVFrame *pFrame, std::string &output);

private:
    bool init(AVFrame *pFrame);
    void release();

    std::string m_taskId;
    ImageType m_imageType;
    AVCodecID m_codecId;
    AVCodecContext *m_codecCtx;
};

#endif
