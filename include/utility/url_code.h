#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

using std::string;

class UrlCodec {
public:
    static unsigned char toHex(const unsigned char &x);

    static unsigned char fromHex(const unsigned char &x);

    static void UrlEncode(const char *sIn, string &strOut);        //编码
    static string UrlDecode(const char *sIn);        //解码

    static bool IsUTF8(const void *pBuffer, long size);

    static bool is_base64(unsigned char c);

    static string urlsafe_b64encode(unsigned char *bytes_to_encode, unsigned int in_len);

    static string base64_encode(const char *bytes_to_encode, unsigned int len);

    static string base64_decode(std::string const &s);

};

UTILITY_NAMESPACE_END