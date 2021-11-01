#pragma once

#include <queue>
#include <mutex>
#include <cstdlib>
#include <event2/event.h>
#include <hiredis/async.h>
#include "core/singleton.h"
#include "core/RingBuffer.h"
#include "Redis.h"
#include "RedisRequest.h"

class RedisRequest;

class RedisAsync {
public:
    RedisAsync(struct event_base *evbase, const std::string &redisPassword);
    ~RedisAsync();

    bool connect(const RedisAddr &addr);
    void close();
    bool working() const { return _stat == Working; }
    void checkPing(time_t now);
    int redisCommand(const std::string &cmdtype, const char *format, ...);
    int redisCommand(RedisRequest *request, const char *format, ...);

private:
    static void redisConnect(const redisAsyncContext *c, int status);
    static void redisDisconnect(const redisAsyncContext *c, int status);
    static void redisCommandCallback(redisAsyncContext *c, void *r, void *privdata);

    // 对应回调
    void onRedisAuth(const redisReply *reply);
    void onRedisPing(const redisReply *reply);

    std::string getRedisPassword(){return m_redisPassword;};
private:
    std::string m_redisPassword;

    struct event_base *_evbase;
    enum {
        NullStat, WaitConn, WaitAuth, Working
    } _stat;
    RedisAddr _addr;
    std::string _addrstr;
    redisAsyncContext *_context;
    time_t _lastPing;
    time_t _lastPingSend;

    struct RequestUri {
        static const std::string RedisUriPing;
        static const std::string RedisUriAuth;
    };
};

class RedisManager : public Singleton<RedisManager> {
    friend class Singleton<RedisManager>;

private:
    RedisManager();
    bool enqueRequest(void *p) { return m_rbIn.write(p); }
    std::string getRedisPassword() const {return m_redisPassword;}

public:
    // 共有函数在外部线程调用
    bool init(const std::vector<std::string> &sentinel, const std::vector<std::string> &scriptPath,
              const std::string &redisMasterName, const std::string &redisPassword);
    bool send(RedisRequest *pRequest);
    bool execute(RedisRequest *request, const char *format, ...);
    std::string getScriptSha1(const std::string &filePath);

private:
    // 以下私有函数只应该在Redis线程调用
    static void *mainLoop(void *arg);
    static void timer(int fd, short event, void *arg);
    static void timer_ms(int fd, short event, void *arg);
    void checkMaster(time_t now);
    bool getMasterAddr();

private:
    // 以下字段初始化之后不会改变，不需要锁
    struct event_base *_evbase;
    RedisAsync *_master;
    std::vector<RedisAddr> _sentinelAddr;
    std::string m_redisMasterName;
    std::string m_redisPassword;

    mutable std::mutex _addrLock;
    RedisAddr _masterAddr;

    // 处理队列，lock free
    RingBuffer m_rbIn;
};
