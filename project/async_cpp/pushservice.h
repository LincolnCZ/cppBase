#ifndef _PUSH_SERVICE_H
#define _PUSH_SERVICE_H

#include <string>
#include <list>
#include <vector>
#include <pthread.h>
#include "config.h"
#include "ISingleton.h"

struct PushMessage {
    std::string hostport;
    std::string traceId;
    std::string result;
    long createTime;
    long nextHop;

    PushMessage() {
        createTime = 0;
        nextHop = 0;
    }
};

class PushService : public ISingleton<PushService> {
    friend class ISingleton<PushService>;

private:
    PushService();

    virtual ~PushService();

    std::vector<std::string> m_servers;
    std::list<PushMessage> m_messages;
    pthread_mutex_t m_mutex;
    std::list<pthread_t> m_threadIdList;

    int flushMsg();

    long getCurrentTimeMilli();

    static void *thread_func(void *arg);

public:
    bool init(int threads);

    bool addMsgToBuffer(std::string &traceid, std::string &server, std::string &result);
};

#endif
