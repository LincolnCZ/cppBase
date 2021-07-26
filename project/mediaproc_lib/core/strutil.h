#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

// 数字转换为字符串
extern char* u32toa(uint32_t value, char* buffer);

inline char* i32toa(int32_t value, char* buffer) {
    uint32_t u = static_cast<uint32_t>(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }

    return u32toa(u, buffer);
}

extern char* u64toa(uint64_t value, char* buffer);

inline char* i64toa(int64_t value, char* buffer) {
    uint64_t u = static_cast<uint64_t>(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }

    return u64toa(u, buffer);
}

template <class Number>
inline std::string to_string(Number n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

template <>
inline std::string to_string(bool n)
{
    return n ? "true" : "false";
}

template <>
inline std::string to_string(char n)
{
    char buf[32];
    char *pos = u32toa((unsigned char)n, buf);
    *pos = '\0';
    return std::string(buf);
}

template <>
inline std::string to_string(unsigned char n)
{
    char buf[32];
    char *pos = u32toa(n, buf);
    *pos = '\0';
    return std::string(buf);
}

template <>
inline std::string to_string(uint32_t n)
{
    char buf[32];
    char *pos = u32toa(n, buf);
    *pos = '\0';
    return std::string(buf);
}

template <>
inline std::string to_string(int32_t n)
{
    char buf[32];
    char *pos = i32toa(n, buf);
    *pos = '\0';
    return std::string(buf);
}

template <>
inline std::string to_string(uint64_t n)
{
    char buf[32];
    char *pos = u64toa(n, buf);
    *pos = '\0';
    return std::string(buf);
}

template <>
inline std::string to_string(int64_t n)
{
    char buf[32];
    char *pos = i64toa(n, buf);
    *pos = '\0';
    return std::string(buf);
}

// 字符串转换为数字
inline long stol(const char *str, bool *succ = NULL)
{
    char *end = NULL;
    long ret = strtol(str, &end, 0);
    if(succ != NULL)
        *succ = (end != str);
    return ret;
}

inline long stol(const std::string &str, bool *succ = NULL)
{
    return stol(str.c_str(), succ);
}

inline unsigned long stoul(const char *str, bool *succ = NULL)
{
    char *end = NULL;
    long ret = strtoul(str, &end, 0);
    if(succ != NULL)
        *succ = (end != str);
    return ret;
}

inline unsigned long stoul(const std::string &str, bool *succ = NULL)
{
    return stoul(str.c_str(), succ);
}

// ip地址和字符串转换
inline std::string addr_ntoa(uint32_t ip)
{
    struct in_addr addr;
    memcpy(&addr, &ip, 4);
    return std::string(::inet_ntoa(addr));
}

inline const char *addr_ntoa_cstr(uint32_t ip)
{
    struct in_addr addr;
    memcpy(&addr, &ip, 4);
    return inet_ntoa(addr);
}

extern char* dumpHex(const void* bin, char* buffer, unsigned int len);
extern std::string dumpHexString(const void *bin, size_t len);

extern void split(const std::string &s, const std::string &delim, std::vector<std::string> &ret);
extern std::string trim(const std::string &str);

inline bool startsWith(const char *str, const char *pattern) {
    return 0 == memcmp(str, pattern, strlen(pattern));
}

inline bool endsWith(const char *str, const char *pattern) {
    int patternLen = strlen(pattern);
    return 0 == memcmp(str+strlen(str)-patternLen, pattern, patternLen);
}

extern void to_upper(std::string &str);
extern void to_lower(std::string &str);

UTILITY_NAMESPACE_END
