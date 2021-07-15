#include "staticservice.h"
#include <unistd.h>
#include "restclient/restclient.h"
#include "logger.h"

StaticService::StaticService() {
    m_serverIndex = 0;
}

StaticService::~StaticService() {

}

bool StaticService::init(std::vector<std::string> serverlist) {
    pthread_mutex_init(&m_mutex, NULL);
    // start a thread
    if (pthread_create(&m_threadId, NULL, thread_func, this) != 0) {
        FUNLOG(Err, "staticservice start thread fail!");
        return false;
    }

    FUNLOG(Info, "staticservice init finish");

    m_servers = serverlist;
    return true;
}

bool StaticService::addMsgProcess(std::string &traceid, std::string &taskid, std::string &requestid) {
    StaticsMessage msg;
    msg.type = MSG_PROCESS;
    msg.taskId = taskid;
    msg.traceId = traceid;
    msg.requestId = requestid;
    msg.result = "";
    msg.cost = 0;

    pthread_mutex_lock(&m_mutex);
    if (m_messages.size() > 100) {
        pthread_mutex_unlock(&m_mutex);
        return false;
    }
    m_messages.push_back(msg);
    pthread_mutex_unlock(&m_mutex);
    return true;
}

bool
StaticService::addMsgComplete(std::string &traceid, std::string &taskid, std::string &requestid, std::string &result,
                              int costMs) {
    StaticsMessage msg;
    msg.type = MSG_COMPLETE;
    msg.taskId = taskid;
    msg.traceId = traceid;
    msg.requestId = requestid;
    msg.result = result;
    msg.cost = costMs;

    pthread_mutex_lock(&m_mutex);
    if (m_messages.size() > 100) {
        pthread_mutex_unlock(&m_mutex);
        return false;
    }
    m_messages.push_back(msg);
    pthread_mutex_unlock(&m_mutex);
    return true;
}

int StaticService::flushMsg() {
    //lock
    pthread_mutex_lock(&m_mutex);
    if (m_messages.size() == 0) {
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }
    StaticsMessage msg = m_messages.front();
    m_messages.pop_front();
    pthread_mutex_unlock(&m_mutex);

    // post to statics service
    if (m_servers.size() > 0) {
        if (m_serverIndex >= m_servers.size()) {
            m_serverIndex = 0;
        }

        bool success = false;
        if (msg.type == MSG_PROCESS) {
            success = sendPostProcess(m_servers[m_serverIndex], msg);
        } else if (msg.type == MSG_COMPLETE) {
            success = sendPostComplete(m_servers[m_serverIndex], msg);
        } else {
            FUNLOG(Err, "unsupport statics type!");
        }

        if (!success) {
            m_serverIndex = (m_serverIndex + 1) % m_servers.size();
            FUNLOG(Err, "send post to statics server fail!");
        }
    }

    return m_messages.size();
}

bool StaticService::sendPostProcess(std::string server, StaticsMessage &msg) {
    char url[512] = {0};
    sprintf(url, "http://%s/process?traceid=%s", server.c_str(), "123");

    // post data struce
    // {"taskId":"porntask2","requestId":"req1"}

    Json::Value root;
    root["taskId"] = msg.taskId;
    root["requestId"] = msg.requestId;
    Json::FastWriter writer;
    std::string data = writer.write(root);
    RestClient::Response resp = RestClient::post(url, "application/json", data.c_str());
    if (200 != resp.code) {
        FUNLOG(Err, "send process to server fail! %s, url=%s, response.code=%d, send body=%s", msg.traceId.c_str(), url,
               resp.code, data.c_str());
        return false;
    }

    FUNLOG(Info, "send process to server success! %s send url=%s, body=%s", msg.traceId.c_str(), url, data.c_str());
    return true;
}

bool StaticService::sendPostComplete(std::string server, StaticsMessage &msg) {
    char url[512] = {0};
    sprintf(url, "http://%s/complete?traceid=%s", server.c_str(), "123");

    // post data struce
    // {"taskId":"porntask2","requestId":"req1","result":"As good as it takes","cost" : 2300}

    Json::Value root;
    root["taskId"] = msg.taskId;
    root["requestId"] = msg.requestId;
    root["result"] = msg.result;
    root["cost"] = msg.cost;

    Json::FastWriter writer;
    std::string data = writer.write(root);
    RestClient::Response resp = RestClient::post(url, "application/json", data.c_str());
    if (200 != resp.code) {
        FUNLOG(Err, "send complete to server fail! %s url=%s, code=%d, send body=%s", msg.traceId.c_str(), url,
               resp.code, data.c_str());
        return false;
    }

    FUNLOG(Info, "send complete to server success! %s send body=%s", msg.traceId.c_str(), data.c_str());
    return true;
}

void *StaticService::thread_func(void *arg) {
    StaticService *instance = (StaticService *) arg;
    while (true) {
        if (0 == instance->flushMsg())
            usleep(100000);
    }
}

