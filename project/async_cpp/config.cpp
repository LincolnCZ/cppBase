#include "config.h"
#include "logger.h"

Config::Config()
{
    m_push_threads = 1;
    m_metrics_service_name = "aladdin_video_swap_face_v2";
}

Config::~Config()
{
}

bool Config::load(const char* config_file)
{
	if(NULL == config_file)
	{
		return false;
	}

    FILE *fp = fopen(config_file, "r");
    if (fp == NULL)
    {
		FUNLOG(Err, "can not open config file: %s", config_file);
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if (fileSize > 512 * 1024)
    {
		FUNLOG(Err, "Config file '%s' too large. size=%ld", config_file, fileSize);
        fclose(fp);
        return false;
    }

	if (fileSize == 0)
	{
		FUNLOG(Err, "Config file '%s' is empty. size=%ld", config_file, fileSize);
		fclose(fp);
		return true;
	}

    char *buf = new char[fileSize + 1];
    size_t n = fread(buf, 1, fileSize, fp);
    fclose(fp);

    buf[fileSize] = 0;
    if (n != fileSize)
    {
        FUNLOG(Err, "Failed to read config file '%s'. fread() return %ld", config_file, n);
        delete []buf;
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    try
    {
		bool ret = reader.parse(buf, root);
		delete[]buf;

        if(!ret)
        {
            FUNLOG(Err, "parse config file failed");
            return  false;
        }

        if(!root["staticservers"].isNull())
        {
            Json::Value serversRoot = root["staticservers"]; 
            for(int x=0; x<serversRoot.size(); x++)
            {
                std::string server = serversRoot[x].asString();
                m_staticservers.push_back(server);
                FUNLOG(Info, "statics server => %s", server.c_str());
            }
        }

        if(root["rabbitmq"].isNull())
        {
            FUNLOG(Err, "'rabbitmq' not found or not an object in config file.");
			return false;
        }


		Json::Value rabbitmqRoot = root["rabbitmq"]; 
        if(rabbitmqRoot["host"].isNull())
        {
            FUNLOG(Err, "'host' not found or not an object in rabbitmq config");
			return false;
        }

        if(rabbitmqRoot["port"].isNull())
        {
            FUNLOG(Err, "'port' not found or not an object in rabbitmq config .\n");
			return false;
        }

        if(rabbitmqRoot["user"].isNull())
        {
            FUNLOG(Err, "'user' not found or not an object in rabbitmq config .\n");
			return false;
        }

        if(rabbitmqRoot["pwd"].isNull())
        {
            FUNLOG(Err, "'pwd' not found or not an object in rabbitmq config .\n");
			return false;
        }

        if(rabbitmqRoot["vhost"].isNull())
        {
            FUNLOG(Err, "'vhost' not found or not an object in rabbitmq config .\n");
			return false;
        }

        if(rabbitmqRoot["queue"].isNull())
        {
            FUNLOG(Err, "'queue' not found or not an object in rabbitmq config .\n");
			return false;
        }

		m_rabbit_host = rabbitmqRoot["host"].asString();
		m_rabbit_port = rabbitmqRoot["port"].asInt();
		m_rabbit_user = rabbitmqRoot["user"].asString();
		m_rabbit_pwd = rabbitmqRoot["pwd"].asString();
		m_rabbit_vhost = rabbitmqRoot["vhost"].asString();
		m_rabbit_queue = rabbitmqRoot["queue"].asString();

        if(root["server"].isNull() || !root["server"].isObject())
        {
            FUNLOG(Err, "'server' not found or not an object in config file.\n");
			return false;
        }

		Json::Value serverRoot = root["server"];

        if(serverRoot["taskid"].isNull())
        {
            FUNLOG(Err, "'taskid' not found or not an object in server config .\n");
			return false;
        }

        if(serverRoot["kafka_topic"].isString())
        {
            m_kafka_topic = serverRoot["kafka_topic"].asString();
        }
        if(serverRoot["kafka_hostport"].isString())
        {
            m_kafka_hostport = serverRoot["kafka_hostport"].asString();
        }

		m_taskid = serverRoot["taskid"].asString();
		int threads = serverRoot["pushthreads"].asInt();
        m_push_threads = threads > 0 ? threads : 1;

        //threads = serverRoot["workthreadsnum"].asInt();
        //m_worker_threads_num = threads > 0 ? threads : 1;

        //m_model_pool_size = 1;
        //if(serverRoot["model_pool_size"].isInt())
        //{
        //    m_model_pool_size = serverRoot["model_pool_size"].asInt();  
        //}

        if(!root["downloadproxy"].isNull() && root["downloadproxy"].isObject())
        {
            Json::Value proxyRoot = root["downloadproxy"];

            if(!proxyRoot["http"].isNull())
            {
                m_download_proxy_http = proxyRoot["http"].asString();
            }

            if(!proxyRoot["https"].isNull())
            {
                m_download_proxy_https = proxyRoot["https"].asString();
            }
        }

        if(!root["metrics"].isNull() && root["metrics"].isObject())
        {
            Json::Value metricsRoot = root["metrics"];
            if(!metricsRoot["servicename"].isNull())
            {
                m_metrics_service_name = metricsRoot["servicename"].asString();
            }
        }

        return true;
    }
    catch(std::exception& e)
    {
        FUNLOG(Err, "got exception '%s' when parse config file\n", e.what());
        return false;
    }

    FUNLOG(Err, "got exception when parse config file\n");
    return false;
}
