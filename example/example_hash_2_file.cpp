#include <iostream>
#include <fstream>
#include <set>
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

int main(int) {
    set<uint32_t> groups{0, 1, 2, 3, 4};
    string line;

    ifstream inf;
    inf.open("./in.txt");
    if (!inf.is_open()) {
        cout << "in file open failed" << endl;
        return 1;
    }

    ofstream outf;
    outf.open("./out.txt");
    if (!outf.is_open()) {
        cout << "out file open failed" << endl;
        return 1;
    }

    //从in.txt　文件中读入数据，并输出到out.txt中
    while (getline(inf, line)) {
        int ret = hashChannelGroup(line, groups);

        set<uint32_t> newGroups{0, 1, 2, 3, 4, 5};
        int new_group = hashChannelGroup(line, newGroups);
        outf << "liveId : " << line << " dm_group:" << ret << ", new_group:" << new_group << '\n';
    }

    inf.close();
    outf.close();
    return 0;
}

