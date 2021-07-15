#include "rabbitmqclient.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>
#include <unistd.h>
#include "logger.h"

void *thread_func(void *arg) {
    RabbitMQClient *client = (RabbitMQClient *) arg;
    if (NULL == client) {
        FUNLOG(Err, "thread func arg is null!");
        return 0;
    }

    int channel = 1;
    while (true) {
        amqp_connection_state_t conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(conn);
        if (!socket) {
            FUNLOG(Err, "amqp_tcp_socket_new fail! ");
            amqp_destroy_connection(conn);
            usleep(1000000);
            continue;
        }

        int status = amqp_socket_open(socket, client->m_host.c_str(), client->m_port);
        if (status) {
            FUNLOG(Err, "amqp_socket_open fail! ");
            amqp_destroy_connection(conn);
            usleep(1000000);
            continue;
        }

        amqp_rpc_reply_t reply = amqp_login(conn, client->m_vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                                            client->m_user.c_str(), client->m_pwd.c_str());
        if (AMQP_RESPONSE_NORMAL != reply.reply_type) {
            FUNLOG(Err, "amqp_login fail!");
            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn);
            usleep(1000000);
            continue;
        }

        amqp_channel_open(conn, channel);
        reply = amqp_get_rpc_reply(conn);
        if (AMQP_RESPONSE_NORMAL != reply.reply_type) {
            FUNLOG(Err, "amqp_channel_open fail!");
            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn);
            usleep(1000000);
            continue;
        }

//		amqp_queue_bind(conn, q->channel, amqp_cstring_bytes(q->queuename), amqp_cstring_bytes(q->exchange),
//		amqp_cstring_bytes(q->bindingkey), amqp_empty_table);
//		reply = amqp_get_rpc_reply(conn);
//		if (AMQP_RESPONSE_NORMAL != reply.reply_type)
//		{
//			  amqp_channel_close(conn, q->channel, AMQP_REPLY_SUCCESS);
//			  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
//			  amqp_destroy_connection(conn);
//			continue; 
//		}

        short prefetchCount = 1;
        amqp_basic_qos(conn, channel, 0, prefetchCount, false);

        amqp_basic_consume(conn, channel, amqp_cstring_bytes(client->m_queue.c_str()), amqp_empty_bytes, 0, 0, 0,
                           amqp_empty_table);
        reply = amqp_get_rpc_reply(conn);
        if (AMQP_RESPONSE_NORMAL != reply.reply_type) {
            FUNLOG(Err, "amqp_basic_consume fail");
            amqp_channel_close(conn, channel, AMQP_REPLY_SUCCESS);
            amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn);
            usleep(1000000);
            continue;
        }

        while (true) {
            try {
                amqp_rpc_reply_t ret;
                amqp_envelope_t envelope;
                amqp_maybe_release_buffers(conn);
                ret = amqp_consume_message(conn, &envelope, NULL, 0);

                if (ret.reply_type == AMQP_RESPONSE_NORMAL) {
                    std::string message;
                    message.assign((const char *) envelope.message.body.bytes, envelope.message.body.len);
                    // callback func
                    (*client->m_callback)(message);

                    //ack msg
                    if (amqp_basic_ack(conn, channel, envelope.delivery_tag, false) > 0) {
                        FUNLOG(Err, "ack message fail!\n");
                    } else {
                        printf("ack msg ok...\n");
                    }
                }
                amqp_destroy_envelope(&envelope);
            }
            catch (...) {
                FUNLOG(Err, "exception! \n");
                break;
            }
        }

        FUNLOG(Err, "amqp_channel_close\n");
        reply = amqp_channel_close(conn, channel, AMQP_REPLY_SUCCESS);
        if (AMQP_RESPONSE_NORMAL != reply.reply_type) {
            usleep(1000000);
            FUNLOG(Err, "close mq channel fail!\n");
        }

        printf("amqp_connection_close\n");
        reply = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        if (AMQP_RESPONSE_NORMAL != reply.reply_type) {
            usleep(1000000);
            FUNLOG(Err, "close mq connection fail!\n");
        }

        FUNLOG(Err, "amqp_destroy_connection\n");
        if (amqp_destroy_connection(conn) < 0) {
            usleep(1000000);
            FUNLOG(Err, "destroy mq connection fail!\n");
        }

        usleep(1000000);
    }
}

RabbitMQClient::RabbitMQClient(std::string host, int port, std::string username, std::string userpwd, std::string vhost,
                               std::string queue, CALLBACK_FUNC func, int threadnum) {
    m_host = host;
    m_port = port;
    m_user = username;
    m_pwd = userpwd;
    m_vhost = vhost;
    m_queue = queue;
    m_callback = func;
    m_threads_num = threadnum;
}

RabbitMQClient::~RabbitMQClient() {
}

// void RabbitMQClient::consum()
// {
// 	int ret = pthread_create(&m_threadId, NULL, thread_func, (void*)this);
// 	if(0 != ret)
// 	{
// 		FUNLOG(Err, "start thread fail!\n");
// 		return;
// 	}

// 	pthread_join(m_threadId, NULL);
// 	FUNLOG(Info, "rabbitmq consum thread exit...\n");
// }

void RabbitMQClient::consum() {
    for (int i = 0; i < m_threads_num; i++) {
        pthread_t threadId;
        int ret = pthread_create(&threadId, NULL, thread_func, (void *) this);
        if (0 != ret) {
            FUNLOG(Err, "start thread fail!\n");
            return;
        }
        m_threadIdList.push_back(threadId);
        FUNLOG(Info, "start rabbitmq consum thread %d...\n", i);
        printf("start rabbitmq consum thread %d...\n", i);
    }


    for (int i = 0; i < m_threadIdList.size(); i++) {
        pthread_join(m_threadIdList[i], NULL);
    }
    FUNLOG(Info, "rabbitmq consum thread exit...\n");
}



