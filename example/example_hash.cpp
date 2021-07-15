#include <set>
#include <iostream>
#include <unistd.h>

using namespace std;

/** hash test **/

inline uint32_t hash_int(uint32_t key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}

inline uint32_t hash_string(uint32_t hash, const char *str) {
    // BKDRHash
    const uint32_t seed = 131;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

uint32_t hashChannelGroup(const string &channel, std::set<uint32_t> groups) {
    uint32_t maxHash = 0;
    uint32_t groupId = 0;
    for (std::set<uint32_t>::iterator it = groups.begin(); it != groups.end(); ++it) {
        uint32_t h = hash_string(*it, channel.c_str());
//        cout << "groupId index: " << *it << "\thash_string :" << h << endl;
        h = hash_int(h);
//        cout << "hash_int :" << h << endl;
        if (h > maxHash) {
            maxHash = h;
            groupId = *it;
        }
    }
    return groupId;
}

int main() {
    set<uint32_t> groups{0, 1, 2, 3, 4};
    string channelId = "1382648146";
    cout << "hash chose : " << hashChannelGroup(channelId, groups);
}