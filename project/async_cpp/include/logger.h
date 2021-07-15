#ifndef EIMD_LOGGER_H
#define EIMD_LOGGER_H

//#include "logging.h"

#include "int_types.h"
#include "syslog-nb.h"
#include <sstream>
#include <string>
#include <sys/time.h>


//extern Logger gLogger;


#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#ifdef __CLASS__
#define FUNLOG(level, fmt, ...)   log(level, "[" __CLASS__ "::%s]: " fmt, __FUNCTION__, ##__VA_ARGS__)
#define STATLOG(level, fmt, ...)   log(level, "[%s::%s]: STAT " fmt, __CLASS__, __FUNCTION__, ##__VA_ARGS__)
#else
#define FUNLOG(level, fmt, ...)   log(level, "[%s:%s:%d]: " fmt, __FILENAME__,  __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define STATLOG(level, fmt, ...)   log(level, "[%s:%s:%d]: STAT " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


enum
{
    Fatal = LOG_EMERG,
    Err = LOG_ERR,
    Warn = LOG_WARNING,
    Notice = LOG_NOTICE,
    Info = LOG_INFO,
    Debug = LOG_DEBUG
};

void init_log();
void log(int  error, const char *fmt, ...);
void vlog(int level, const char* fmt, va_list vl);
void setLogLevel(int level);
int  getLogLevel();

char *ip2str(uint32_t ip);
char *time2str(uint32_t t);
char *uri2str(uint32_t uri);
std::string bin2hex(const char *bin, uint32_t len);
char *int642str(uint64_t num);
char *int2str(uint32_t num);





#endif // LOGGER_H

