#include "logger.h"
//#include "logging.h"

//! Decouple default reportability level for sample and TRT-API specific logging.
//! This change is required to not log TRT-API specific info by default.
//! To enable verbose logging of TRT-API use `setReportableSeverity(Severity::kINFO)`.

//! TODO: Revert gLoggerSample to gLogger to use same reportablilty level for TRT-API and samples
//! once we have support for Logger::Severity::kVERBOSE. TensorRT runtime will enable this
//! new logging level in future releases when making other ABI breaking changes.

//! gLogger is used to set default reportability level for TRT-API specific logging.
//Logger gLogger{Logger::Severity::kWARNING};


//======================================================================================
int syslog_level = Info;
//======================================================================================

void setLogLevel(int level)
{
    log(Notice, "set log level %d", level);
    syslog_level = level;
}
int getLogLevel()
{
    return syslog_level;
}

//======================================================================================
void init_log()
{
    openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
}
//======================================================================================
void log(int l, const char *fmt, ...)
{
    if (l > syslog_level)
        return;

    va_list		param;

    va_start(param, fmt);
#ifdef __USE_BSD
    vsyslog_nb(l, fmt, param);
#else
    syslog_nb_t(l, fmt, param);
#endif
    va_end(param);
}
//======================================================================================
void vlog(int l, const char *fmt, va_list param)
{
    if (l > syslog_level)
        return;
#ifdef __USE_BSD
    vsyslog_nb(l, fmt, param);
#else
    syslog_nb_t(l, fmt, param);
#endif
}
//======================================================================================

char *ip2str(uint32_t ip)
{
    union ip_addr{
        uint32_t addr;
        uint8_t s[4];
    } a;
    a.addr = ip;
    static char s[16];
    sprintf(s, "%u.%u.%u.%u", a.s[0], a.s[1], a.s[2], a.s[3]);
    return s;
}

char *time2str(uint32_t t)
{
    static char buffer[64] = "";

    time_t xx = t;
    struct tm* local = localtime(&xx);
    if (local != NULL)
    {
        strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", local);
    }
    else
    {
        sprintf(buffer, "%s", "0000-00-00 00:00:00");
    }
    return buffer;
}

char *uri2str(uint32_t uri)
{
    uint32_t uri_h = uri/256;
    uint32_t uri_t = uri % 256;

    static char s[16];
    sprintf(s, "%3d|%3d", uri_h, uri_t);
    return s;
}


char *int2str(uint32_t num)
{
    static char s[12];
    sprintf(s, "%u", num);
    return s;
}


char *int642str(uint64_t num)
{
    static char s[22];
#if defined(__x86_64__)
    sprintf(s, "%ld", num);
#else
    sprintf(s, "%lld", num);
#endif
    return s;
}

std::string bin2hex(const char *bin, uint32_t len){
    std::ostringstream os;
    for(uint32_t i = 0; i<len; i++){
        char st[4];
        uint8_t c = bin[i];
        sprintf(st, "%02x ", c);
        os << st;
    }
    return os.str();
}

//======================================================================================
//end

