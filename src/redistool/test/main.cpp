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
        return RedisManager::getInstance()->execute(this, "EVALSHA %s 1 %s", sha1.c_str(), "test");
    }

    bool OnReplyImpl(redisAsyncContext *, const redisReply *reply) override {
        if (reply->type != REDIS_REPLY_STRING) {
            FUNCLOG (Warn, "%s: invalid reply type:%d", getRequestType().c_str(), reply->type);
            return false;
        }

        std::cout << reply->str << std::endl;

        return true;
    }

    bool OnCompletedImpl() override { return false; }

private:
    std::string m_filePth;
};

int main() {
    init_log();

    std::string luaScript("./test.lua");
    vector<string> sentinels{"49.7.17.186:17371", "58.215.169.58:17371", "58.220.51.219:17371",
                             "157.255.234.164:17371"};
    vector<string> scriptPaths{luaScript};

    string redisMasterName("linemaster");
    string redisPassword("BY2pEfnzvb");

    RedisManager::getInstance()->init(sentinels, scriptPaths, redisMasterName, redisPassword);

    sleep(3);

    RedisRequest *request = new TestRedisRequest(luaScript);
    RedisManager::getInstance()->send(request);

    while (true) {}
}

