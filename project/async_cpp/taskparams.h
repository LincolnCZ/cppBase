#include <string>
#include <string.h>
#include <vector>
#include <json/json.h>

struct UserPic {
    std::string url;
    Json::Value meta_data;
    std::string celeb_name;
};
struct InputConfig {
    Json::Value context;        // context will send on callback

    std::string url;
    std::string dstfilename;
};

enum UPLOAD_TYPE {
    UPLOAD_BS2 = 0,
    UPLOAD_OSS,     // aliyun oss
    UPLOAD_GOOGLE,  // google storage
};
enum PUSH_TYPE {
    PUSH_HTTP = 0,
    PUSH_KAFKA,
};
struct OutputConfig {
    UPLOAD_TYPE upload_type;   // bs2 or oss or google
    std::string cdn_host;

    // for google auth
    std::string google_auth;

    // for bs2 or oss
    std::string bucket;
    std::string secret_key;
    std::string secret_code;
    std::string end_point;

    PUSH_TYPE push_type;        // http or kafka
    std::string http_url;       // for http callback
    std::string kafka_hostport; // for kafka callback
    std::string kafka_topic;    // for kafka callback
};

class TaskParams {
public:
    TaskParams();

    ~TaskParams();

    bool parse(std::string &jsonstr);

    std::string m_traceid;
    std::string m_secretid;
    int m_priority;
    std::string m_taskid;
    std::vector<InputConfig> m_inputs;
    OutputConfig m_output;
};
