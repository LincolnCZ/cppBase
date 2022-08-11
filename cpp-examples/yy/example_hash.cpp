#include <set>
#include <iostream>
#include <unistd.h>

using namespace std;

/** Thomas Wang's 32 bit Mix Function 算法 **/
// 算法说明：https://www.cnblogs.com/napoleon_liu/archive/2010/12/29/1920839.html
//http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
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

uint32_t hashChannelGroup(const string &channel, const std::set<uint32_t> &groups) {
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

int main() {
    set<uint32_t> groups{0, 1, 2, 3, 4};
    string channelId = "2806828531"; //    87103145
    cout << "hash chose : " << hashChannelGroup(channelId, groups);
}