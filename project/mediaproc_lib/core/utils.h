#pragma once
#include <string>
#include "util_common.h"
#include "strutil.h"
#include "maputil.h"

UTILITY_NAMESPACE_BEGIN

template <typename Function>
class DeferAction
{
    Function _func;

public:
    DeferAction(Function &f) : _func(f) {}
    ~DeferAction() { _func(); }
};

// defer 的调用顺序满足FILO
template <typename Function>
DeferAction<Function> defer(Function f) {
    return DeferAction<Function>(f);
}

// get monotonic time
extern int64_t getClockUS();
extern int64_t getClockMS();

extern bool writeFile(const char *path, const std::string &data);
extern bool readFile(const char *path, std::string &data);

UTILITY_NAMESPACE_END