#include "core/logger.h"
#include "syslog-nb.h"

int syslog_level = Info;

void init_log()
{
    openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
}
