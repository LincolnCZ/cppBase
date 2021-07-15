#include <string>

class ossclient {
public:
    ossclient(const std::string &access_key, const std::string &access_secret, const std::string &endpoint);

    ~ossclient();

    int upload(const std::string &local_file, const std::string &save_name, const std::string &bucket,
               std::string &ret_file_url, std::string &err_msg);

private:
    std::string m_access_key;
    std::string m_access_secret;
    std::string m_endpoint;
};
