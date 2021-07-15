#pragma once
#include <string>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

// get monotonic time
extern uint64_t getClockS(bool real = false);
extern uint64_t getClockMS(bool real = false);
extern uint64_t getClockUS(bool real = false);
extern uint64_t getClockNS(bool real = false);

extern bool writeFile(const std::string &path, const std::string &data);
extern bool readFile(const std::string &path, std::string &data);

UTILITY_NAMESPACE_END