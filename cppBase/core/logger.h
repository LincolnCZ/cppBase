#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

// _log do not check level, only print
inline const char *_get_filename(const char *path) {
    const char *file = strrchr(path, '/');
    return (file == NULL) ? path : file + 1;
}

extern void syslog_nb(int __pri, const char *__fmt, ...)
__attribute__ ((__format__ (__printf__, 2, 3)));

enum {
    Fatal = LOG_EMERG,
    Error = LOG_ERR,
    Warn = LOG_WARNING,
    Notice = LOG_NOTICE,
    Info = LOG_INFO,
    Debug = LOG_DEBUG
};

#ifndef LOGGER_USING_PRINT
#define LOG(level, fmt, args...) do{if((level) <= getLogLevel()) syslog_nb(level, fmt, ##args);}while(0)
#else
#define LOG(level, fmt, args...) do{if((level) <= getLogLevel()) printf("[%d] " fmt "\n", level, ##args);}while(0)
#endif

#ifdef __CLASS__
#define FUNCLOG(level, fmt, args...)   LOG(level, "[" __CLASS__ "::%s]: " fmt, __FUNCTION__, ##args)
#define STATLOG(level, fmt, args...)   LOG(level, "[" __CLASS__ "::%s]: STAT " fmt, __FUNCTION__, ##args)
#else
#define FUNCLOG(level, fmt, args...)   LOG(level, "[%s:%s]: " fmt, _get_filename(__FILE__), __FUNCTION__, ##args)
#define STATLOG(level, fmt, args...)   LOG(level, "[%s:%s]: STAT " fmt, _get_filename(__FILE__), __FUNCTION__, ##args)
#endif

extern int syslog_level;

void init_log();

static inline void setLogLevel(int level) { syslog_level = level; }

static inline int getLogLevel() { return syslog_level; }

#ifdef __cplusplus
}
#endif
