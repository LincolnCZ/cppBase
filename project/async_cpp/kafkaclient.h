#ifndef _KAFKA_CLIENT_H
#define _KAFKA_CLIENT_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <unistd.h>

/*
 * Typical include path in a real application would be
 * #include <librdkafka/rdkafkacpp.h>
 */
#include "librdkafka/rdkafkacpp.h"
#include "logger.h"


class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message &message) {
        /* If message.err() is non-zero the message delivery failed permanently
         * for the message. */
        if (message.err())
            std::cerr << "% Message delivery failed: " << message.errstr() << std::endl;
        else
            std::cerr << "% Message delivered to topic " << message.topic_name() <<
                      " [" << message.partition() << "] at offset " <<
                      message.offset() << std::endl;
    }
};

class CKafkaClient {
public:
    CKafkaClient(const std::string &traceid, const std::string &brokers, const std::string &topic,
                 const std::string &data, int push_retry_times = 3);

    ~CKafkaClient();

    bool sendMessage();

private:
    std::string m_traceid;
    std::string m_brokers;
    std::string m_topic;
    std::string m_data;
    int m_push_retry_times;
    RdKafka::Conf *m_conf;
    //RdKafka::Producer *m_producer;

    ExampleDeliveryReportCb m_ex_dr_cb;
};


#endif
