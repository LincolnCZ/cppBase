#define __CLASS__ "RedisManager"

#include <algorithm>
#include <iostream>
#include <sys/prctl.h>
#include <cassert>
#include <event2/event_struct.h>
#include <hiredis/adapters/libevent.h>
#include "core/logger.h"
#include "utility/strutil.h"
#include "utility/sysutil.h"

#include "redistool/RedisManager.h"

#define REDIS_PING_INTERVAL 5
#define REDIS_PING_TIMEOUT 15

#ifndef MAX_RING_BUFFER_SLOT_COUNT
#define MAX_RING_BUFFER_SLOT_COUNT (8 * 1024)
#endif

#define UNUSED(x) (void)(x)


class CheckTimeOutUtil {
public:
    CheckTimeOutUtil(uint32_t interval) : m_lastTimeOutTimeStamp(0), m_timeOutInterval(interval) {}

    CheckTimeOutUtil(uint32_t interval, uint32_t now) : m_lastTimeOutTimeStamp(now), m_timeOutInterval(interval) {}

    bool isTimeOut(uint32_t now) { return m_lastTimeOutTimeStamp + m_timeOutInterval < now; }

    bool checkSetTimeout(uint32_t now) {
        if (m_lastTimeOutTimeStamp + m_timeOutInterval < now) {
            m_lastTimeOutTimeStamp = now;
            return true;
        } else
            return false;
    }

    void setLastTimeOutTimeStamp(uint32_t now) { m_lastTimeOutTimeStamp = now; }

private:
    uint32_t m_lastTimeOutTimeStamp;
    uint32_t m_timeOutInterval;
};

//--------------------------------RedisAsync----------------------------------
const std::string RedisAsync::RequestUri::RedisUriPing("redisPing");
const std::string RedisAsync::RequestUri::RedisUriAuth("redisAuth");

RedisAsync::RedisAsync(struct event_base *evbase, const std::string &redisPassword) {
    assert(evbase != NULL);
    assert(scriptMgr != NULL);

    m_redisPassword = redisPassword;

    _evbase = evbase;
    _stat = NullStat;
    _context = NULL;
    _lastPing = 0;
    _lastPingSend = 0;
}

RedisAsync::~RedisAsync() {
    close();
}

bool RedisAsync::connect(const RedisAddr &addr) {
    if (addr.empty())
        return false;
    _addr = addr;
    _addrstr = addr.ip + ":" + utility::to_string(addr.port);

    _context = redisAsyncConnect(_addr.ip.c_str(), _addr.port);
    if (_context->err) {
        FUNCLOG(Warn, "connect %s:%u error: %s",
                _addr.ip.c_str(), _addr.port, _context->errstr);
        redisAsyncFree(_context);
        _context = NULL;
        return false;
    }

    _stat = WaitConn;
    _context->data = this;
    redisLibeventAttach(_context, _evbase);
    redisAsyncSetConnectCallback(_context, RedisAsync::redisConnect);
    redisAsyncSetDisconnectCallback(_context, RedisAsync::redisDisconnect);
    return true;
}

void RedisAsync::close() {
    FUNCLOG(Notice, "close redis %s, stat %d", _addrstr.c_str(), _stat);
    if (_context != NULL) {
        redisAsyncFree(_context);
        _context = NULL;
    }
    _stat = NullStat;

    // 清理script脚本
    LoadScriptMgr::getInstance()->clearSha1();
}

void RedisAsync::checkPing(time_t now) {
    if (_stat != Working)
        return;

    if (_lastPing + REDIS_PING_TIMEOUT < now) {
        FUNCLOG(Warn, "redis ping timeout, addr %s", _addrstr.c_str());
        close();
        return;
    }

    if (_lastPingSend + REDIS_PING_INTERVAL < now) {
        redisCommand(RequestUri::RedisUriPing, "PING");
        _lastPingSend = now;
    }
}

void RedisAsync::redisConnect(const redisAsyncContext *c, int status) {
    RedisAsync *mgr = reinterpret_cast<RedisAsync *>(c->data);
    assert(c == mgr->_context);

    if (status != REDIS_OK) {
        FUNCLOG(Warn, "redis connect error: %s", c->errstr);
        // hiredis连接失败，会清空对应context
        mgr->_stat = NullStat;
        mgr->_context = NULL;
        return;
    }

    FUNCLOG(Notice, "connected redis %s", mgr->_addrstr.c_str());
    mgr->_stat = WaitAuth;
    mgr->redisCommand(RequestUri::RedisUriAuth, "AUTH %s", mgr->getRedisPassword().c_str());

    LoadScriptMgr::getInstance()->load(utility::getClockMS()); // todo
}

void RedisAsync::redisDisconnect(const redisAsyncContext *c, int status) {
    RedisAsync *mgr = reinterpret_cast<RedisAsync *>(c->data);
    assert(c == mgr->_context);

    FUNCLOG(Notice, "disconnect redis %s, status %d", mgr->_addrstr.c_str(), status);
    // hiredis连接关闭接口,会清空对应context
    mgr->_stat = NullStat;
    mgr->_context = NULL;
}

int RedisAsync::redisCommand(const std::string &cmdtype, const char *format, ...) {
    va_list ap;
    int status;
    RedisRequest *request = new RedisRequest(cmdtype);
    va_start(ap, format);
    status = redisvAsyncCommand(_context, RedisAsync::redisCommandCallback,
                                reinterpret_cast<void *>(request), format, ap);
    va_end(ap);
    return status;
}

int RedisAsync::redisCommand(RedisRequest *request, const char *format, ...) {
    va_list ap;
    int status;
    va_start(ap, format);
    status = redisvAsyncCommand(_context, RedisAsync::redisCommandCallback,
                                reinterpret_cast<void *>(request), format, ap);
    va_end(ap);
    return status;
}

void RedisAsync::redisCommandCallback(redisAsyncContext *c, void *r, void *privdata) {
    RedisRequest *request = reinterpret_cast<RedisRequest *>(privdata);
    redisReply *reply = reinterpret_cast<redisReply *>(r);
    RedisAsync *mgr = reinterpret_cast<RedisAsync *>(c->data);

    if (!request) {
        FUNCLOG (Error, "redis command : NULL request, redis:%s",
                 mgr->_addr.toString().c_str());
        return;
    }

    if (!reply) {
        FUNCLOG (Error, "redis requestType:%s, traceId:%s: NULL reply, err:%s, redis:%s",
                 request->getRequestType().c_str(), request->getTraceId().c_str(), c ? c->errstr : "",
                 mgr->_addr.toString().c_str());
    }

    FUNCLOG(Info, "redisCommandCallback request type:%s, traceId:%s", request->getRequestType().c_str(),
            request->getTraceId().c_str());

    // handle internal redis commands
    if (request->getRequestType() == RequestUri::RedisUriPing) {
        mgr->onRedisPing(reply);
        delete request;
        return;
    } else if (request->getRequestType() == RequestUri::RedisUriAuth) {
        mgr->onRedisAuth(reply);
        delete request;
        return;
    } else if (request->getRequestType().empty()) {
        FUNCLOG (Warn, "unknown redis requestType:%s, traceId:%s, addr %s",
                 request->getRequestType().c_str(), request->getTraceId().c_str(), mgr->_addr.toString().c_str());
        delete request;
        return;
    }

    // parse reply in redis thread context
    request->OnReply(c, reply);

    delete request;
}

void RedisAsync::onRedisAuth(const redisReply *reply) {
    if (reply->type == REDIS_REPLY_ERROR) {
        FUNCLOG(Warn, "redis auth error: %s", reply->str);
        return;
    }

    FUNCLOG(Notice, "redis auth success: %s", reply->str);
    _lastPing = time(NULL);
    _stat = Working;
}


void RedisAsync::onRedisPing(const redisReply *reply) {
    if (reply->type == REDIS_REPLY_ERROR) {
        FUNCLOG(Warn, "redis recv ping error: %s", reply->str);
        return;
    }
    FUNCLOG(Info, "redis recv ping success: %s", reply->str);
    _lastPing = time(NULL);
}

//--------------------------------RedisManager----------------------------------

RedisManager::RedisManager() :
        m_rbIn("RS_INPUT_Q", MAX_RING_BUFFER_SLOT_COUNT) {
    _evbase = NULL;
    _master = NULL;
}

bool RedisManager::init(const std::vector<std::string> &sentinel, const std::vector<std::string> &scriptPath,
                        const std::string &redisMasterName, const std::string &redisPassword) {

    // init in queue
    if (!m_rbIn.initialize()) {
        FUNCLOG (Error, "fail to initialize in queue");
        return false;
    }

    m_redisMasterName = redisMasterName;
    m_redisPassword = redisPassword;

    if (!LoadScriptMgr::getInstance()->init(scriptPath)) {
        FUNCLOG(Error, "init m_scriptMgr failed");
        return false;
    }

    for (size_t i = 0; i < sentinel.size(); i++) {
        RedisAddr addr = RedisAddr::parseString(sentinel[i]);
        if (addr.empty()) {
            FUNCLOG(Warn, "parse redis addr %s error", sentinel[i].c_str());
            return false;
        }
        _sentinelAddr.push_back(addr);
    }
    if (!getMasterAddr()) {
        FUNCLOG(Warn, "fail to get master redis");
        return false;
    }

    pthread_t pthread;
    int ret = pthread_create(&pthread, NULL, RedisManager::mainLoop, this);
    if (ret != 0) {
        FUNCLOG(Warn, "fail to create new thread");
        return false;
    }
    return true;
}

bool RedisManager::execute(RedisRequest *request, const char *format, ...) {

    int status = _master->redisCommand(request, format);

    return (status == REDIS_OK);
}

bool RedisManager::send(RedisRequest *pRequest) {
    bool ret = false;

    if (!pRequest) {
        FUNCLOG (Error, "NULL request");
        return ret;
    }

    if (!enqueRequest(pRequest)) {
        FUNCLOG (Error, "%s: in queue full, traceId:%s", pRequest->getRequestType().c_str(),
                 pRequest->getTraceId().c_str());
        delete pRequest;
    }
    ret = true;

    return ret;
}

std::string RedisManager::getScriptSha1(const std::string &filePath) {
    return LoadScriptMgr::getInstance()->getSha1(filePath);
}

void *RedisManager::mainLoop(void *arg) {
    prctl(PR_SET_NAME, "RedisWrite");
    RedisManager *mgr = reinterpret_cast<RedisManager *>(arg);
    mgr->_evbase = event_base_new();

    struct timeval tv = {1, 0};
    struct event tmev;
    event_assign(&tmev, mgr->_evbase, -1, EV_PERSIST, RedisManager::timer, &tmev);
    evtimer_add(&tmev, &tv);

    struct timeval tv_100ms = {0, 100000};
    struct event tmev_ms;
    event_assign(&tmev_ms, mgr->_evbase, -1, EV_PERSIST, RedisManager::timer_ms, &tmev_ms);
    evtimer_add(&tmev_ms, &tv_100ms);

    mgr->_master = new RedisAsync(mgr->_evbase, mgr->getRedisPassword());
    if (!mgr->_master->connect(mgr->_masterAddr)) {
        FUNCLOG(Warn, "connect master fail");
        exit(-1);
    }

    event_base_dispatch(mgr->_evbase);
    return NULL;
}

void RedisManager::timer(int fd, short event, void *arg) {
    UNUSED(fd);
    UNUSED(event);

    RedisManager *mgr = RedisManager::getInstance();
    time_t now = time(NULL);
    mgr->checkMaster(now);
    mgr->_master->checkPing(now);

    struct timeval tv = {1, 0};
    struct event *timeout = reinterpret_cast<struct event *>(arg);
    event_add(timeout, &tv);
}

void RedisManager::timer_ms(int fd, short event, void *arg) {
    UNUSED(fd);
    UNUSED(event);
    RedisManager *mgr = RedisManager::getInstance();
    if (mgr->_master == NULL || !mgr->_master->working()) {
        return;
    }

    // 定期检查加载script脚本，防止connect的时候加载失败
    LoadScriptMgr::getInstance()->load(utility::getClockMS());

    // 处理队列中的请求
    void *pRaw = NULL;
    while (mgr->m_rbIn.read(&pRaw)) {
        RedisRequest *pRequest = static_cast<RedisRequest *>(pRaw);
        if (pRequest) {
            if (!pRequest->Execute()) {
                FUNCLOG(Error, "pRequest execute failed;");
//                 return to main thread if attach fail
//                if (!onResponse(pRequest)) {
//                     miserable, bury this pool little creature...
//                    delete pRequest;
//                }
            }
        }
        pRaw = NULL;
    }

    struct timeval tv_100ms = {0, 100000};
    struct event *timeout = reinterpret_cast<struct event *>(arg);
    event_add(timeout, &tv_100ms);
}


void RedisManager::checkMaster(time_t now) {
    static CheckTimeOutUtil check(30);
    if (_master->working())
        return;
    if (!check.checkSetTimeout(now))
        return;

    char buf[256];
    snprintf(buf, sizeof(buf), "redis master disconnect, addr %s:%d", _masterAddr.ip.c_str(), _masterAddr.port);
    FUNCLOG(Error, "%s", buf);
//    sendAlarm(buf); // todo

    _master->close();
    if (!getMasterAddr())
        return;
    _master->connect(_masterAddr);
}

bool RedisManager::getMasterAddr() {
    std::lock_guard<std::mutex> lock(_addrLock);
    std::random_shuffle(_sentinelAddr.begin(), _sentinelAddr.end());
    for (size_t i = 0; i < _sentinelAddr.size(); i++) {
        RedisAddr &addr = _sentinelAddr[i];
        RedisSentinel sentinel(m_redisMasterName);
        if (!sentinel.connect(addr.ip.c_str(), addr.port, 200)) {
            FUNCLOG(Warn, "connect sentinel %s:%u error", addr.ip.c_str(), addr.port);
            continue;
        }
        if (!sentinel.getRedisMaster(&_masterAddr)) {
            FUNCLOG(Warn, "get master from sentinel %s:%u error", addr.ip.c_str(), addr.port);
            continue;
        } else {
            FUNCLOG(Info, "set master %s:%u from sentinel %s:%u",
                    _masterAddr.ip.c_str(), _masterAddr.port, addr.ip.c_str(), addr.port);
            return true;
        }
    }
    return false;
}
