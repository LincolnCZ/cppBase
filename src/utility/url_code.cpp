#include "utility/url_code.h"
#include <ctype.h>
#include <stdio.h>

UTILITY_NAMESPACE_BEGIN

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static const std::string urlsafe_base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789-_";

unsigned char UrlCodec::toHex(const unsigned char &x) {
    return x > 9 ? x - 10 + 'A' : x + '0';
}

unsigned char UrlCodec::fromHex(const unsigned char &x) {
    return isdigit(x) ? x - '0' : x - 'A' + 10;
}

void UrlCodec::UrlEncode(const char *sIn, std::string &strOut) {
    size_t len = strlen(sIn);
    unsigned char buf[4];
    for (size_t ix = 0; ix < len; ix++) {
        memset(buf, 0x00, 4);
        if (isalnum((unsigned char) sIn[ix])) {
            buf[0] = sIn[ix];
        } else {
            buf[0] = '%';
            buf[1] = toHex((unsigned char) sIn[ix] >> 4);
            buf[2] = toHex((unsigned char) sIn[ix] % 16);
        }
        strOut += (char *) buf;
    }
    return;
}

std::string UrlCodec::UrlDecode(const char *sIn) {
    std::string sOut;
    size_t len = strlen(sIn);
    unsigned char ch = 0;
    for (size_t ix = 0; ix < len; ix++) {
        ch = 0;
        if (sIn[ix] == '%') {
            ch = (fromHex(sIn[ix + 1]) << 4);
            ch |= fromHex(sIn[ix + 2]);
            ix += 2;
        } else if (sIn[ix] == '+') {
            ch = ' ';
        } else {
            ch = sIn[ix];
        }
        sOut += (char) ch;
    }

    return sOut;
}

bool UrlCodec::IsUTF8(const void *pBuffer, long size) {
    bool IsUTF8 = true;
    unsigned char *start = (unsigned char *) pBuffer;
    unsigned char *end = (unsigned char *) pBuffer + size;
    while (start < end) {
        if (*start < 0x80) // (10000000): 值小于0x80的为ASCII字符
        {
            start++;
        } else if (*start < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
        {
            IsUTF8 = false;
            break;
        } else if (*start < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符
        {
            if (start >= end - 1)
                break;
            if ((start[1] & (0xC0)) != 0x80) {
                IsUTF8 = false;
                break;
            }
            start += 2;
        } else if (*start < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符
        {
            if (start >= end - 2)
                break;
            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80) {
                IsUTF8 = false;
                break;
            }
            start += 3;
        } else {
            IsUTF8 = false;
            break;
        }
    }
    return IsUTF8;
}

#if 0
#endif

bool UrlCodec::is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string UrlCodec::urlsafe_b64encode(unsigned char *bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += urlsafe_base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += urlsafe_base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

// base64编码
std::string UrlCodec::base64_encode(const char *bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    char char_array_3[3];
    char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

// base64 解码
std::string UrlCodec::base64_decode(std::string const &encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}

UTILITY_NAMESPACE_END;