#include "kafkaclient.h"

CKafkaClient::CKafkaClient(const std::string &traceid, const std::string &brokers, const std::string &topic,
                           const std::string &data, int push_retry_times) :
        m_traceid(traceid), m_brokers(brokers), m_topic(topic), m_data(data), m_push_retry_times(push_retry_times) {
    m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
}

CKafkaClient::~CKafkaClient() {
    if (m_conf != NULL) {
        delete m_conf;
    }
}

bool CKafkaClient::sendMessage() {
    std::string errstr;
    if (m_data.empty()) {
        FUNLOG(Err, "traceid=%s, the push data is empty", m_traceid.c_str());
        return false;
    }

    /* Set bootstrap broker(s) as a comma-separated list of
    * host or host:port (default port 9092).
    * librdkafka will use the bootstrap brokers to acquire the full
    * set of brokers from the cluster. */
    if (m_conf->set("bootstrap.servers", m_brokers, errstr) !=
        RdKafka::Conf::CONF_OK) {
        // std::cerr << errstr << std::endl;
        FUNLOG(Err, "%s", errstr.c_str());

        return false;
    }

    /* Set the delivery report callback.
    * This callback will be called once per message to inform
    * the application if delivery succeeded or failed.
    * See dr_msg_cb() above.
    * The callback is only triggered from ::poll() and ::flush().
    *
    * IMPORTANT:
    * Make sure the DeliveryReport instance outlives the Producer object,
    * either by putting it on the heap or as in this case as a stack variable
    * that will NOT go out of scope for the duration of the Producer object.
    */
    if (m_conf->set("dr_cb", &m_ex_dr_cb, errstr) != RdKafka::Conf::CONF_OK) {
        // std::cerr << errstr << std::endl;
        FUNLOG(Err, "traceid=%s, err: %s", m_traceid.c_str(), errstr.c_str());
        return false;
    }

    /*
    * Create producer instance.
    */
    RdKafka::Producer *m_producer = RdKafka::Producer::create(m_conf, errstr);
    if (!m_producer) {
        FUNLOG(Err, "traceid=%s, Failed to create producer: %s", m_traceid.c_str(), errstr.c_str());
        // std::cerr << "Failed to create producer: " << errstr << std::endl;
        return false;
    }

    m_producer->poll(0);
    for (int i = 0; i < m_push_retry_times; i++) {
        /*
            * Send/Produce message.
            * This is an asynchronous call, on success it will only
            * enqueue the message on the internal producer queue.
            * The actual delivery attempts to the broker are handled
            * by background threads.
            * The previously registered delivery report callback
            * is used to signal back to the application when the message
            * has been delivered (or failed permanently after retries).
            */
        RdKafka::ErrorCode err =
                m_producer->produce(
                        /* Topic name */
                        m_topic,
                        /* Any Partition: the builtin partitioner will be
                            * used to assign the message to a topic based
                            * on the message key, or random partition if
                            * the key is not set. */
                        RdKafka::Topic::PARTITION_UA,
                        /* Make a copy of the value */
                        RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                        /* Value */
                        const_cast<char *>(m_data.c_str()), m_data.size(),
                        /* Key */
                        NULL, 0,
                        /* Timestamp (defaults to current time) */
                        0,
                        /* Message headers, if any */
                        NULL,
                        /* Per-message opaque value passed to
                            * delivery report */
                        NULL);

        if (err != RdKafka::ERR_NO_ERROR) {
            FUNLOG(Err, "traceid=%s, Failed to produce to topic: %s, err:%s", m_traceid.c_str(), m_topic.c_str(),
                   RdKafka::err2str(err).c_str());
            // std::cerr << "% Failed to produce to topic " << m_topic << ": " <<
            //     RdKafka::err2str(err) << std::endl;
            m_producer->poll((i + 1) * 2000);//increase max block time
            continue;
        } else {
            FUNLOG(Info, "traceid=%s, FEnqueued message(%d bytes) for topic:%s", m_traceid.c_str(), m_data.size(),
                   m_topic.c_str());
            // std::cerr << "% Enqueued message (" << m_data.size() << " bytes) " <<
            // "for topic " << m_topic << std::endl;
            break;
        }
    }

    /* Wait for final messages to be delivered or fail.
    * flush() is an abstraction over poll() which
    * waits for all messages to be delivered. */
    // std::cerr << "% Flushing final messages..." << std::endl;
    FUNLOG(Info, "traceid=%s, Flushing final messages...", m_traceid.c_str());
    m_producer->flush(10 * 1000 /* wait for max 10 seconds */);

    if (m_producer->outq_len() > 0) {
        FUNLOG(Info, "traceid=%s, message(s) were not delivered, m_producer outq_len: %d", m_traceid.c_str(),
               m_producer->outq_len());
        delete m_producer;
        return false;
    }
    delete m_producer;
    return true;
}


// int main (int argc, char **argv) {

//     if (argc != 3) {
//         std::cerr << "Usage: " << argv[0] << " <brokers> <topic>\n";
//         exit(1);
//     }

//     std::string brokers = argv[1];
//     std::string topic = argv[2];

//     std::string traceid= "test";
//     std::string pushdata="hello world!!!";
//     CKafkaClient client(traceid, brokers, topic, pushdata);
//     bool success = client.sendMessage();
//     if(success)
//     {
//         std::cout << "push to kafka success" << std::endl;
//     }
//     else{
//         std::cout << "push to kafka failed" << std::endl;
//     }

//     return 0;
// }

