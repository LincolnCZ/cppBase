#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <json/json.h>

class Config
{
	public:
		Config();
		~Config();
		bool load(const char* config_file);
	
		std::string m_rabbit_host;
		int m_rabbit_port;
		std::string m_rabbit_user;
		std::string m_rabbit_pwd;
		std::string m_rabbit_vhost;
		std::string m_rabbit_queue;

        std::vector<std::string> m_staticservers;

        std::string m_download_proxy_http;
        std::string m_download_proxy_https;

		std::string m_taskid;
        int m_push_threads;
		//int m_worker_threads_num;

        std::string m_metrics_service_name;
        std::string m_kafka_hostport; 
        std::string m_kafka_topic;

        // video classify model size
        //int m_model_pool_size;
};
#endif

