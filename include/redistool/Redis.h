#pragma once

#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include "hiredis/hiredis.h"

struct RedisAddr {
    std::string ip;
    int port;

    RedisAddr() : port(0) {}
    RedisAddr(const char *ip_, int port_) : ip(ip_), port(port_) {}
    bool empty() const { return ip.empty() || port == 0; }

    void clear() {
        ip.clear();
        port = 0;
    }

    std::string toString() const {
        std::ostringstream os;
        os << ip << ":" << port;
        return os.str();
    }

    static RedisAddr parseString(const std::string &str);
};

class RedisSentinel {
public:
    explicit RedisSentinel(const std::string& redisMasterName);
    ~RedisSentinel();

    bool connect(const char *ip, int port, int timeout = 0);
    void close();
    bool getRedisMaster(RedisAddr *addr);
    bool getRedisSlave(std::vector<RedisAddr> &slaves);

private:
    redisContext *_context;
    std::string m_redisMasterName;
};
