#ifndef _RABBITMQ_CLIENT_H
#define _RABBITMQ_CLIENT_H

#include <pthread.h>
#include <string>
#include <vector>

typedef void (*CALLBACK_FUNC)(std::string &data);

class RabbitMQClient {
public:
    RabbitMQClient(std::string host, int port, std::string username, std::string userpwd, std::string vhost,
                   std::string queue, CALLBACK_FUNC func, int threadnum);

    ~RabbitMQClient();

    void consum();

    pthread_t m_threadId;
    std::string m_host;
    int m_port;
    std::string m_user;
    std::string m_pwd;
    std::string m_vhost;
    std::string m_queue;

    CALLBACK_FUNC m_callback;

    int m_threads_num;
    std::vector<pthread_t> m_threadIdList;
};

#endif
