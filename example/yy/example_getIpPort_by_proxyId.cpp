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
//    uint64_t proxyId = 0x58bb7d7b2356119e; // 743 064 103 258 889 648
    uint64_t proxyId = 3632686216163958192;
    uint32_t port = proxyId & 0x0ffff;
    uint32_t ip = proxyId >> 32;
    cout << "ip:port = " << addr_ntoa(ip) << ":" << port << endl;
}