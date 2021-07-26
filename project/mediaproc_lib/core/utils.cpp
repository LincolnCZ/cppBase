#include "utils.h"
#include <fstream>
#include <vector>
#include <time.h>

UTILITY_NAMESPACE_BEGIN

int64_t getClockMS() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
	return (int64_t)(ts.tv_sec * 1000L + ts.tv_nsec / 1000000L);
}

int64_t getClockUS() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
	return (int64_t)(ts.tv_sec * 1000000L + ts.tv_nsec / 1000L);
}

bool writeFile(const char *path, const std::string &data)
{
    std::ofstream ofs(path, std::ofstream::binary | std::ios::trunc);
    if(!ofs.is_open())
        return false;
    ofs << data;
    ofs.close();
    return true;
}

bool readFile(const char *path, std::string &data)
{
    std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
    if(!ifs.is_open())
        return false;
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);
    data.assign(bytes.data(), fileSize);
    return true;
}

UTILITY_NAMESPACE_END