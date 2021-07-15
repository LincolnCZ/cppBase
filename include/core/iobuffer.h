#pragma once
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>

class IOBuffer
{
private:
    char *buf;
    size_t cap;
    size_t readpos;
    size_t writepos;

    IOBuffer(const IOBuffer & o);
    IOBuffer &operator=(const IOBuffer& o);

public:
    IOBuffer(size_t n = 256)
    {
        buf = (char*)malloc(n);
        cap = n;
        readpos = 0;
        writepos = 0;
    }

    ~IOBuffer() 
    {
        if(buf != NULL)
            free(buf);
    }

    const void *data() const
    {
        return buf + readpos;
    }

    size_t size() const
    {
        return writepos - readpos;
    }

    void has_read(size_t s)
    {
        readpos += s;
        assert(readpos <= writepos);
    }

    void *write_data()
    {
        return buf + writepos;
    }

    size_t write_size()
    {
        return cap - writepos;
    }

    void has_write(size_t s)
    {
        writepos += s;
        assert(writepos <= cap);
    }

    void append(const void* data, size_t size)
    {
        if(writepos + size > cap) {
            grow(size);
        }
        memcpy(buf + writepos, data, size);
        writepos += size;
    }

    // grow 增长缓冲区长度，保证至少有传入的可写入空间
    void grow(size_t n)
    {
        if(readpos > 0) {
            if(writepos > readpos) {
                memmove(buf, buf+readpos, writepos-readpos);
            }
            writepos -= readpos;
            readpos = 0;
        }
        if(writepos + n > cap) {
            size_t ns = std::max(writepos + n, cap * 2);
            buf = (char*)realloc(buf, ns);
            cap = ns;
        }
    }

    void clear()
    {
        readpos = 0;
        writepos = 0;
    }
};