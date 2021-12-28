#include <iostream>
#include "core/logger.h"
#include "redistool/RedisManager.h"
#include "redistool/RedisRequest.h"
#include <unistd.h>

using namespace std;

// <sentinel>49.7.17.186:17371</sentinel>
// <sentinel>58.215.169.58:17371</sentinel>
// <sentinel>58.220.51.219:17371</sentinel>
// <sentinel>157.255.234.164:17371</sentinel>

class TestRedisRequest : public RedisRequest {
public:
    TestRedisRequest(const std::string &filePath) : RedisRequest(filePath), m_filePth(filePath) {
    }

    ~TestRedisRequest() override = default;


    bool ExecuteImpl() override {
        string sha1 = RedisManager::getInstance()->getScriptSha1(m_filePth);
        cout << "test.lua sha1 : " << sha1 << endl;
        return RedisManager::getInstance()->execute(this, "EVALSHA %s 1 %ld", sha1.c_str(), 10034);
    }

    bool OnReplyImpl(redisAsyncContext *, const redisReply *reply) override {
        if (reply->type != REDIS_REPLY_ARRAY) {
            FUNCLOG (Warn, "%s: invalid reply type:%d", getRequestType().c_str(), reply->type);
            return false;
        }

        std::ostringstream oss;
        oss << "str:" << std::string(reply->str, reply->len) << ", type:" << reply->type << ",elements:"
            << reply->elements;
        oss << ",element:[";
        for (int i = 0; i < reply->elements; ++i) {
            redisReply *r = reply->element[i];
            oss << "{ str:" << std::string(r->str, r->len) << ", type:" << r->type << ",integer:" << r->integer << "}";
        }
        oss << "]";
        std::cout << "lua script res:" << oss.str() << std::endl;

        return true;
    }

    bool OnCompletedImpl() override { return false; }

private:
    std::string m_filePth;
};

int main() {
    init_log();

    std::string luaScript("./query.lua");
    vector<string> sentinels{"183.36.111.150:17371", "106.120.191.79:17371", "221.228.209.198:17371"};
    vector<string> scriptPaths{luaScript};

    string redisMasterName("linemaster");
    string redisPassword("BY2pEfnzvb");

    RedisManager::getInstance()->init(sentinels, scriptPaths, redisMasterName, redisPassword);

    sleep(3);

    RedisRequest *request = new TestRedisRequest(luaScript);
    RedisManager::getInstance()->send(request);

    while (true) {}
}

