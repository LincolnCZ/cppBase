#include <cassert>
#include <cstring>
#include <map>

#include "core/logger.h"
#include "utility/strutil.h"

#include "redistool/Redis.h"

using namespace std;

map<string, string> redisParseMap(const redisReply *reply) {
    map<string, string> ret;
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
        string key = reply->element[i]->str;
        string val = reply->element[i + 1]->str;
        ret[key] = val;
    }
    return ret;
}

RedisAddr RedisAddr::parseString(const string &str) {
    RedisAddr ret;
    size_t idx = str.find(':');
    if (idx == std::string::npos)
        return ret;
    ret.ip = str.substr(0, idx);
    ret.port = utility::stoul(str.substr(idx + 1));
    return ret;
}

RedisSentinel::RedisSentinel(const std::string& redisMasterName) {
    _context = NULL;
    m_redisMasterName = redisMasterName;
}

RedisSentinel::~RedisSentinel() {
    close();
}

bool RedisSentinel::connect(const char *ip, int port, int timeout) {
    assert(_context == NULL);
    if (timeout == 0)
        _context = redisConnect(ip, port);
    else {
        struct timeval tm = {timeout / 1000, (timeout % 1000) * 1000};
        _context = redisConnectWithTimeout(ip, port, tm);
    }

    if (_context == NULL || _context->err) {
        if (_context == NULL)
            FUNCLOG(Warn, "allocate redis context error");
        else
            FUNCLOG(Warn, "redis connect %s:%d, error %s", ip, port, _context->errstr);
        return false;
    }
    return true;
}

void RedisSentinel::close() {
    if (_context) {
        redisFree(_context);
        _context = NULL;
    }
}

bool RedisSentinel::getRedisMaster(RedisAddr *addr) {
    assert(_context != NULL);
    redisReply *reply;
    reply = (redisReply *) redisCommand(_context, "SENTINEL get-master-addr-by-name %s", m_redisMasterName.c_str());
    if (reply == NULL) {
        FUNCLOG(Warn, "get redis master fail: %s", _context->errstr);
        return false;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
        FUNCLOG(Warn, "get redis master type %d error %s", reply->type, reply->str);
        return false;
    }

    addr->ip = reply->element[0]->str;
    addr->port = atoi(reply->element[1]->str);

    freeReplyObject(reply);
    return true;
}

bool RedisSentinel::getRedisSlave(std::vector<RedisAddr> &slaves) {
    assert(_context != NULL);
    slaves.clear();

    redisReply *reply;
    reply = (redisReply *) redisCommand(_context, "SENTINEL slaves %s", m_redisMasterName.c_str());
    if (reply == NULL) {
        FUNCLOG(Warn, "get redis slave fail: %s", _context->errstr);
        return false;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
        FUNCLOG(Warn, "get redis slaves type %d error %s", reply->type, reply->str);
        return false;
    }

    for (size_t i = 0; i < reply->elements; i++) {
        map<string, string> slavemap = redisParseMap(reply->element[i]);
        if (slavemap["flags"] == "slave") {
            RedisAddr addr;
            addr.ip = slavemap["ip"];
            addr.port = atoi(slavemap["port"].c_str());
            FUNCLOG(Info, "get slave ip %s port %u", addr.ip.c_str(), addr.port);
            slaves.push_back(addr);
        } else {
            FUNCLOG(Warn, "slave %s flags error %s", slavemap["name"].c_str(), slavemap["flags"].c_str());
        }
    }

    freeReplyObject(reply);
    return true;
}
