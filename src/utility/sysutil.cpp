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

uint32_t get_ip(const char *hostname) {
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    int ret = getaddrinfo(hostname, NULL, &hints, &result);
    if (ret != 0) {
        return 0;
    }
    uint32_t ip = ((sockaddr_in *) result->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(result);

    return ip;
}

UTILITY_NAMESPACE_END