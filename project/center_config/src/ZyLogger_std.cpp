#include <iostream>
#include "ZyLogger.h"

using namespace std;

enum ZyLogger::ZyLogLevel ZyLogger::ms_outputLevel = ZyLogger::TRACE_LOG;

ZyLogger::ZyLogger(enum ZyLogLevel level) :
        m_level(level) {
}

ZyLogger::~ZyLogger() {
    if (m_level < ms_outputLevel)
        return;
    switch (m_level) {
        case TRACE_LOG:
            cout << "[TRACE]" << m_stream.str();
            cout << endl;
            break;
        case DEBUG_LOG:
            cout << "[DEBUG]" << m_stream.str();
            cout << endl;
            break;
        case WARN_LOG:
            cout << "[WARN]" << m_stream.str();
            cout << endl;
            break;
        case ERROR_LOG:
            cerr << "[ERROR]" << m_stream.str();
            cout << endl;
            break;
    }
}

ZyLogger &ZyLogger::operator<<(const char *msg) {
    if (m_level >= ms_outputLevel)
        m_stream << msg;
    return *this;
}

void ZyLogger::setLogOutputLevel(enum ZyLogLevel outLevel) {
    ms_outputLevel = outLevel;
}
