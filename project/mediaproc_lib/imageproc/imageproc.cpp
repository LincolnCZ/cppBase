#include "imageproc.h"
#include <stdlib.h>
#include <assert.h>
#include "core/logger.h"
#include "anim_util.h"
#include "image_enc.h"

class WriteMemoryFile
{
public:
    WriteMemoryFile() {
        buf_ = nullptr;
        size_ = 0;
        fp_ = nullptr;
    }

    ~WriteMemoryFile() {
        if(fp_ != nullptr) {
            fclose(fp_);
            fp_ = nullptr;
        }
        if(buf_ != nullptr) {
            free(buf_);
            buf_ = nullptr;
        }
    }
    WriteMemoryFile(const WriteMemoryFile&) = delete;
    WriteMemoryFile &operator=(const WriteMemoryFile&) = delete;

    FILE *open() {
        assert(fp_ == nullptr);
        fp_ = open_memstream(&buf_, &size_);
        return fp_;
    }

    void close() {
        assert(fp_ != nullptr);
        fclose(fp_);
        fp_ = nullptr;
    }

    const char *data() const {
        return buf_;
    }

    size_t size() const {
        return size_;
    }

private:
    char *buf_;
    size_t size_;
    FILE *fp_;
};

int splitImageWebp(const char *file, const std::string &input, std::vector<std::string> &imgs)
{
    imgs.clear();

    // 解码 webp 图片
    WebPData webp_data;
    WebPDataInit(&webp_data);
    uint8_t* file_data = (uint8_t*)WebPMalloc(input.size());
    memcpy(file_data, input.data(), input.size());
    webp_data.bytes = file_data;
    webp_data.size = input.size();

    AnimatedImage image;
    memset(&image, 0, sizeof(image));
    if(!ReadAnimatedImage(file, &webp_data ,&image)) {
        FUNLOG(Warn, "%s, decode webp image fail", file);
        WebPDataClear(&webp_data);
        return 1;
    }
    WebPDataClear(&webp_data);

    // 编码每一帧结果到png格式
    int result = 0;
    for(uint32_t i = 0; i < image.num_frames; ++i) {
        WebPDecBuffer buffer;
        WebPInitDecBuffer(&buffer);
        buffer.colorspace = MODE_RGBA;
        buffer.is_external_memory = 1;
        buffer.width = image.canvas_width;
        buffer.height = image.canvas_height;
        buffer.u.RGBA.rgba = image.frames[i].rgba;
        buffer.u.RGBA.stride = buffer.width * sizeof(uint32_t);
        buffer.u.RGBA.size = buffer.u.RGBA.stride * buffer.height;

        WriteMemoryFile wfile;
        if(!WebPWritePNG(wfile.open(), &buffer)) {
            FUNLOG(Error, "%s, write png index %d fail", file, i);
            wfile.close();
            WebPFreeDecBuffer(&buffer);
            result = 1;
            break;
        }
        wfile.close();
        imgs.push_back(std::string(wfile.data(), wfile.size()));

        WebPFreeDecBuffer(&buffer);
    }
    ClearAnimatedImage(&image);
    return result;
}