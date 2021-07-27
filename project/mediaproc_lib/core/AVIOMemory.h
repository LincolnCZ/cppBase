#pragma once
#include <stdint.h>
#include <string>
extern "C" {
#include <libavformat/avformat.h>
}

// 只读内存模拟
// 不保存所读取内容，因此不需要进行内存拷贝。使用input内存必须在其使用过程中一直存在
class AVIOReadMemory {
public:
    AVIOReadMemory(const char *input, int size);
    ~AVIOReadMemory();
    AVIOReadMemory(const AVIOReadMemory&) = delete;
    AVIOReadMemory &operator=(const AVIOReadMemory&) = delete;

    AVIOContext *get() {
        return avio;
    }

private:
    static int readString(void *opaque, uint8_t *buf, int size);
    static int64_t seekString(void *opaque, int64_t offset, int whence);

    AVIOContext *avio;
    const char *buffer;
    int total;
    int pos;
};

class AVIOMemory {
public:
    AVIOMemory();
    ~AVIOMemory();
    AVIOMemory(const AVIOMemory&) = delete;
    AVIOMemory &operator=(const AVIOMemory&) = delete;

    void reset(const char *input, int size);
    AVIOContext *get() { return avio; }
    const std::string &getBuffer() {return buffer;}

private:
    static int readString(void *opaque, uint8_t *buf, int size);
    static int writeString(void *opaque, uint8_t *buf, int size);
    static int64_t seekString(void *opaque, int64_t offset, int whence);

    AVIOContext *avio;
    std::string buffer;
    int pos;
};
