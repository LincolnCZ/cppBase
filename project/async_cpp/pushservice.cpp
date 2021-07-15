#include "pushservice.h"
#include <unistd.h>
#include "restclient/restclient.h"
#include "logger.h"
#include "common.h"
#include "include/MetricsEx.h"

PushService::PushService() {
}

PushService::~PushService() {
}

long PushService::getCurrentTimeMilli() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec) / 1000;
}

bool PushService::init(int threads) {
    pthread_mutex_init(&m_mutex, NULL);

    // start a thread
    for (int i = 0; i < threads; i++) {
        pthread_t threadId;
        if (pthread_create(&threadId, NULL, thread_func, this) != 0) {
            FUNLOG(Err, "pushservice start thread fail!");
            return false;
        }

        FUNLOG(Info, "new push thread success! index=%d", i);
        m_threadIdList.push_back(threadId);
    }

    FUNLOG(Info, "pushservice init finish");
    return true;
}

bool PushService::addMsgToBuffer(std::string &traceid, std::string &server, std::string &result) {
    PushMessage msg;
    msg.hostport = server;
    msg.traceId = traceid;
    msg.result = result;
    msg.createTime = getCurrentTimeMilli() / 1000;

    pthread_mutex_lock(&m_mutex);
    if (m_messages.size() > 1000) {
        pthread_mutex_unlock(&m_mutex);
        return false;
    }

    m_messages.push_back(msg);
    pthread_mutex_unlock(&m_mutex);
    return true;
}

int PushService::flushMsg() {
    //lock
    pthread_mutex_lock(&m_mutex);
    if (m_messages.size() == 0) {
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    PushMessage msg = m_messages.front();
    m_messages.pop_front();
    FUNLOG(Info, "%s get from buffer", msg.traceId.c_str());

    bool repush = false;
    if ((msg.createTime + msg.nextHop) > getCurrentTimeMilli() / 1000) {
        repush = true;
        m_messages.push_back(msg);
        FUNLOG(Info, "%s repush to buffer, msg.createTime=%ld, nextHop=%ld, currentTime=%ld", msg.traceId.c_str(),
               msg.createTime, msg.nextHop, getCurrentTimeMilli() / 1000);
    }
    pthread_mutex_unlock(&m_mutex);

    if (repush) {
        return 0;
    }

    int ret = PROCESS_ERROR_OK;
    {
        MetricsAutoReporter report("i", &ret, "push/http");
        FUNLOG(Info, "%s try to push result to http server: %s", msg.traceId.c_str(), msg.hostport.c_str());
        for (int x = 0; x < 3; x++) {
            RestClient::Response resp = RestClient::post(msg.hostport.c_str(), "application/json", msg.result.c_str());
            if (200 != resp.code) {
                FUNLOG(Err, "%s http push fail! result=%s, code=%d", msg.traceId.c_str(), msg.result.c_str(),
                       resp.code);
                usleep(500000);
                continue;
            }

            ret = PROCESS_ERROR_OK;
            FUNLOG(Info, "%s http push success! data=%s", msg.traceId.c_str(), msg.result.c_str());
            break;
        }
    }

    if (PROCESS_ERROR_OK != ret) {
        // push back to list
        if (msg.nextHop < 300) {
            msg.nextHop += 15;

            pthread_mutex_lock(&m_mutex);
            FUNLOG(Info, "%s repush to message list", msg.traceId.c_str());
            m_messages.push_back(msg);
            pthread_mutex_unlock(&m_mutex);
        }
    }

    return m_messages.size();
}

void *PushService::thread_func(void *arg) {
    PushService *instance = (PushService *) arg;
    while (true) {
        if (0 == instance->flushMsg())
            usleep(100000);
    }
}

