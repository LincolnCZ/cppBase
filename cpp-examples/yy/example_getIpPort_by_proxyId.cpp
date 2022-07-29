#include <iostream>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

inline std::string addr_ntoa(u_long ip) {
    struct in_addr addr;
    memcpy(&addr, &ip, 4);
    return std::string(inet_ntoa(addr));
}

int main() {
    uint64_t proxyId = 351457574938 ;
    uint16_t port = proxyId & 0xffff;
    uint16_t listenport = (proxyId >> 16) & 0xffff;
    uint32_t ip = proxyId >> 32;
    cout << "ip:port = " << addr_ntoa(ip) << ":" << port << ", listenport:" << listenport << endl;

    uint64_t conn_id = ((uint64_t) ip << 32) |
                       ((uint64_t) listenport << 16) | (uint64_t) port;

    long uri = 1019 << 8 | 80;
    cout << uri << endl;
    long t = 261200 >> 8;
    cout << t << endl;
    cout << hex << 1734186066270297520 << endl;

    typedef unsigned short uint16;
    uint16 uport = 7900;
    printf("%u, %lld\n", uport, htons(uport));
    cout << hex<< 0x4b2c150065 << endl;
}