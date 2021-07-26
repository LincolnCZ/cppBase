#include "AVIOMemory.h"
#include <algorithm>

AVIOReadMemory::AVIOReadMemory(const char *input, int size) {
    buffer = input;
    total = size;
    pos = 0;

    int bufferSize = 8 * 1024; // 8KB
    unsigned char *readBuffer = (unsigned char*)av_malloc(bufferSize);
    avio = avio_alloc_context(readBuffer, bufferSize, 0, this, readString, NULL, seekString);
}

AVIOReadMemory::~AVIOReadMemory() {
    if(avio) {
        av_freep(&avio->buffer);
        av_freep(&avio);
    }
}

int AVIOReadMemory::readString(void *opaque, uint8_t *buf, int size) {
    AVIOReadMemory *self = reinterpret_cast<AVIOReadMemory*>(opaque);
    int left = self->total - self->pos;
    if(left == 0)
        return AVERROR_EOF;
    int bytesRead = std::min(left, size); 
    memcpy(buf, self->buffer + self->pos, bytesRead);
    self->pos += bytesRead;
    return bytesRead;
}

int64_t AVIOReadMemory::seekString(void *opaque, int64_t offset, int whence) {
    AVIOReadMemory *self = reinterpret_cast<AVIOReadMemory*>(opaque);
    int64_t new_pos = 0;
    switch (whence)
    {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = self->pos + offset;
        break;
    case SEEK_END:
        new_pos = self->total + offset;
        break;
    case AVSEEK_SIZE:   // 返回文件大小，但不改变位置
        return self->total;
    default:
        return -1;
    }
    // 针对 seek 位置超出文件范围，linux的lseek允许这种情况下继续写入
    // 在此处实现中，直接返回失败
    if(new_pos > self->total || new_pos < 0)
        return -1;

    self->pos = std::min((int)new_pos, self->total);
    return new_pos;
}

AVIOMemory::AVIOMemory() {
    avio = NULL;
    pos = 0;
    int bufferSize = 8 * 1024; // 8KB
    unsigned char *readBuffer = (unsigned char*)av_malloc(bufferSize);
    avio = avio_alloc_context(readBuffer, bufferSize, 1, this, readString, writeString, seekString);
}

AVIOMemory::~AVIOMemory() {
    if(avio) {
        av_freep(&avio->buffer);
        av_freep(&avio);
    }
}

void AVIOMemory::reset(const char *input, int size) {
    buffer.assign(input, size);
    pos = 0;
}

int AVIOMemory::readString(void *opaque, uint8_t *buf, int size) {
    AVIOMemory *self = reinterpret_cast<AVIOMemory*>(opaque);
    int left = self->buffer.size() - self->pos;
    if(left == 0)
        return AVERROR_EOF;
    int bytesRead = std::min(left, size); 
    memcpy(buf, self->buffer.data() + self->pos, bytesRead);
    self->pos += bytesRead;
    return bytesRead;
}

int AVIOMemory::writeString(void *opaque, uint8_t *buf, int size) {
    AVIOMemory *self = reinterpret_cast<AVIOMemory*>(opaque);
    int left = self->buffer.size() - self->pos;
    if(left <= size) {
        if(left != 0)
            self->buffer.erase(self->pos, std::string::npos);
        self->buffer.append((const char*)buf, size);
    } else {
        self->buffer.replace(self->pos, size, (const char*)buf, size);
    }

    self->pos += size;
    return size;
}

int64_t AVIOMemory::seekString(void *opaque, int64_t offset, int whence) {
    AVIOMemory *self = reinterpret_cast<AVIOMemory*>(opaque);
    int64_t new_pos = 0;
    int64_t buffer_size = self->buffer.size();
    switch (whence)
    {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = self->pos + offset;
        break;
    case SEEK_END:
        new_pos = buffer_size + offset;
        break;
    case AVSEEK_SIZE:   // 返回文件大小，但不改变位置
        return buffer_size;
    default:
        return -1;
    }
    // 针对 seek 位置超出文件范围，linux的lseek允许这种情况下继续写入
    // 在此处实现中，直接返回失败
    if(new_pos > buffer_size || new_pos < 0)
        return -1;

    self->pos = std::min(new_pos, buffer_size);
    return new_pos;
}
