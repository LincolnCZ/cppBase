#ifndef _ZY_LOG_H_
#define _ZY_LOG_H_

#include <sstream>
#include <iostream>

using namespace std;

#define ZLOGTRACE  ZyLogger(ZyLogger::TRACE_LOG)
#define ZLOGDEBUG  ZyLogger(ZyLogger::DEBUG_LOG)
#define ZLOGWARN   ZyLogger(ZyLogger::WARN_LOG)
#define ZLOGERROR  ZyLogger(ZyLogger::ERROR_LOG)

class ZyLogger {
public:
    enum ZyLogLevel {
        TRACE_LOG,
        DEBUG_LOG,
        WARN_LOG,
        ERROR_LOG,
    };

public:
    ZyLogger(enum ZyLogLevel level);

    template<class T>
    ZyLogger &operator<<(const T &msg) {
        if (m_level >= ms_outputLevel)
            m_stream << msg;
        return *this;
    }

    ZyLogger &operator<<(const char *msg);

    ~ZyLogger();

    static void setLogOutputLevel(enum ZyLogLevel);

private:
    enum ZyLogLevel m_level;
    ostringstream m_stream;
    static enum ZyLogLevel ms_outputLevel;
};

#endif