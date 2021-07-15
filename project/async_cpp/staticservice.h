#ifndef _STATIC_SERVICE_H
#define _STATIC_SERVICE_H

#include <string>
#include <list>
#include <vector>
#include <pthread.h>
#include "config.h"
#include "ISingleton.h"

enum MsgType {
    MSG_PROCESS = 0,
    MSG_COMPLETE = 1
};

struct StaticsMessage {
    MsgType type;
    std::string taskId;
    std::string traceId;
    std::string requestId;
    std::string result;
    int cost;

    StaticsMessage() {
        cost = 0;
    }
};

class StaticService : public ISingleton<StaticService> {
    friend class ISingleton<StaticService>;

private:
    StaticService();

    virtual ~StaticService();

    std::vector<std::string> m_servers;
    std::list<StaticsMessage> m_messages;
    int m_serverIndex;
    pthread_mutex_t m_mutex;
    pthread_t m_threadId;

    bool sendPostProcess(std::string server, StaticsMessage &msg);

    bool sendPostComplete(std::string server, StaticsMessage &msg);

    int flushMsg();

    static void *thread_func(void *arg);

public:
    bool init(std::vector<std::string> serverlist);

    bool addMsgProcess(std::string &traceid, std::string &taskid, std::string &requestid);

    bool
    addMsgComplete(std::string &traceid, std::string &taskid, std::string &requestid, std::string &result, int costMs);
};

#endif
