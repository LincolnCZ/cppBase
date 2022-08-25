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
    uint64_t proxyId = 0x1270c87133d217d7  ;
    uint16_t port = proxyId & 0xffff;
    uint16_t listenport = (proxyId >> 16) & 0xffff;
    uint32_t ip = proxyId >> 32;
    cout << "ip:port = " << addr_ntoa(ip) << ":" << port << ", listenport:" << listenport << endl;
}