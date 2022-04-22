#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_EVENT_NUMBER 1024

static int SetNonBlock(int iFd) {
    int iFLValue = 0;

    if ((iFLValue = fcntl(iFd, F_GETFL, 0)) == -1) {
        return -1;
    }

    return fcntl(iFd, F_SETFL, iFLValue | O_NONBLOCK);
}

int AsyncConnect(int clientFd) {
    int epollFd = epoll_create(5);  //Event table size 5
    if (epollFd == -1) {
        printf("fail to create epoll!\n");
        return -1;
    }

    struct epoll_event events[MAX_EVENT_NUMBER];

    // 添加网络套接口文件描述符到epoll中
    struct epoll_event xEvent;
    xEvent.data.fd = clientFd;
    xEvent.events = EPOLLOUT | EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &xEvent);

    // 等待epoll通知
    int ret = epoll_wait(epollFd, events, MAX_EVENT_NUMBER, -1);
    if (ret < 0) {
        printf("epoll failure!\n");
        return -1;
    }

    for (int i = 0; i < ret; i++) {
        if (events[i].events & EPOLLIN) {
            std::cout << "EPOLLIN" << std::endl;
        } else if (events[i].events & EPOLLOUT) {
            std::cout << "EPOLLOUT, 连接成功" << std::endl;
            //First reading and writing
            char str1[] = "hello!";
            write(clientFd, str1, sizeof(str1));

            sleep(5);
        } else {
            printf("something unexpected happened!\n");
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    // 创建socket套接口
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);

    // 设置为非阻塞套接口
    SetNonBlock(clientFd);

    //2.连接服务器
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(8888);
    for (;;) {
        int ret = connect(clientFd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
        if (ret == 0) {
            std::cout << "connect to server successfully." << std::endl;
            close(clientFd);
            return 0;
        } else if (ret == -1) {
            if (errno == EINTR) {
                //connect 动作被信号中断，重试connect
                std::cout << "connecting interrupted by signal, try again." << std::endl;
                continue;
            } else if (errno == EINPROGRESS) {
                //进入异步模式
                if (AsyncConnect(clientFd) == -1) {
                    ::close(clientFd);     // 如果失败，记得关闭套接字
                }
                return 0;
            } else {
                //真的出错了，
                close(clientFd);
                return -1;
            }
        }
    }
}