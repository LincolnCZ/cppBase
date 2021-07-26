#include "logger.h"

int syslog_level = Info;

void setLogLevel(int level)
{
    syslog_level = level;
}

void init_log()
{
    openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
}
