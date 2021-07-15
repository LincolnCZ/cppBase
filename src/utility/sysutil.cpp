#include "utility/sysutil.h"
#include <fstream>
#include <vector>
#include <time.h>

UTILITY_NAMESPACE_BEGIN


uint64_t getClockS(bool real) {
    struct timespec ts;
    if (real) {
        clock_gettime(CLOCK_REALTIME, &ts);
    } else {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    return (uint64_t) (ts.tv_sec);
}

uint64_t getClockMS(bool real) {
    struct timespec ts;
    if (real) {
        clock_gettime(CLOCK_REALTIME, &ts);
    } else {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    return (uint64_t) (ts.tv_sec * 1000L + ts.tv_nsec / 1000000L);
}

uint64_t getClockUS(bool real) {
    struct timespec ts;
    if (real) {
        clock_gettime(CLOCK_REALTIME, &ts);
    } else {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    return (uint64_t) (ts.tv_sec * 1000000L + ts.tv_nsec / 1000L);
}

uint64_t getClockNS(bool real) {
    struct timespec ts;
    if (real) {
        clock_gettime(CLOCK_REALTIME, &ts);
    } else {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    return (uint64_t) (ts.tv_sec * 1000000000LL + ts.tv_nsec);
}

bool writeFile(const std::string &path, const std::string &data) {
    std::ofstream ofs(path, std::ofstream::binary | std::ios::trunc);
    if (!ofs.is_open())
        return false;
    ofs << data;
    ofs.close();
    return true;
}

bool readFile(const std::string &path, std::string &data) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!ifs.is_open())
        return false;
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);
    data.assign(bytes.data(), fileSize);
    return true;
}

UTILITY_NAMESPACE_END