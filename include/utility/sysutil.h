#pragma once

#include <string>
#include <cstring>
#include <netdb.h>
#include <set>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

/** --------------- get monotonic time --------------- */
extern uint64_t getClockS(bool real = false);
extern uint64_t getClockMS(bool real = false);
extern uint64_t getClockUS(bool real = false);
extern uint64_t getClockNS(bool real = false);

/** --------------- 读写文件 --------------- */
extern bool writeFile(const std::string &path, const std::string &data);
extern bool readFile(const std::string &path, std::string &data);

extern uint32_t get_ip(const char *hostname);
inline uint32_t get_ip(const std::string &host) { return utility::get_ip(host.c_str()); }

/** --------------- 哈希算法 --------------- */
/** Thomas Wang's 32 bit Mix Function 算法 **/
// 算法说明：https://www.cnblogs.com/napoleon_liu/archive/2010/12/29/1920839.html
// http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
// https://www.jianshu.com/p/bb64cd7593ab
inline uint32_t hash_int(uint32_t key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}

/**BKDRHash 算法*/
// https://byvoid.com/zhs/blog/string-hash-compare/
inline uint32_t hash_string(uint32_t hash, const char *str) {
    const uint32_t seed = 131;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

uint32_t hashChannelGroup(const std::string &channel, const std::set<uint32_t> &groups) {
    uint32_t maxHash = 0;
    uint32_t groupId = 0;
    for (std::set<uint32_t>::iterator it = groups.begin(); it != groups.end(); ++it) {
        uint32_t h = hash_string(*it, channel.c_str());
        h = hash_int(h);
        if (h > maxHash) {
            maxHash = h;
            groupId = *it;
        }
    }
    return groupId;
}

UTILITY_NAMESPACE_END