#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<bits/stdc++.h>
#include<arpa/inet.h>

int main(int argc, char *argv[]) {
    addrinfo hints, *res, *p;
    int ret;
    char ipstr[INET6_ADDRSTRLEN];

    /* if (argc != 2)
    {
        std::cout << "bad input\n";
        exit(-1);
    }*/

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    ret = getaddrinfo(argv[1], "80", &hints, &res);

    if (ret) {
        fprintf(stderr, "%s", gai_strerror(ret));
        exit(-1);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            inet_ntop(p->ai_family,
                      &(((struct sockaddr_in *) (p->ai_addr))->sin_addr),
                      ipstr, sizeof(ipstr));
            std::cout << "IPV4: " << ipstr << '\n';
        } else {
            inet_ntop(p->ai_family,
                      &(((struct sockaddr_in6 *) (p->ai_addr))->sin6_addr),
                      ipstr, sizeof(ipstr));
            std::cout << "IPV6: " << ipstr << '\n';
        }
    }

    freeaddrinfo(res);
}
// 测试：
// ./test google.com
//IPV4: 172.217.27.142
//IPV6: 2404:6800:4012::200e

// ./test apple.com
//IPV4: 17.172.224.47
//IPV4: 17.142.160.59
//IPV4: 17.178.96.59

// ./test www.baidu.com
//IPV4: 182.61.200.7
//IPV4: 182.61.200.6