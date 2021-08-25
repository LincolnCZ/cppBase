#include    "base_api.h"
#include    <sys/resource.h>//getrlimit获取RLIMIT_NOFILE
#include    <poll.h>

#define INFTIM -1
#define OPEN_MAX 1024

int main(int argc, char **argv) {
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLINE];
    socklen_t clilen;
    //获取当前Linux操作系统下，一个进程能打开的最大文件数目
    struct rlimit limit;
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
        err_sys("getrlimit");
    const int OPEN_MAX_ = (int) limit.rlim_cur;
    struct pollfd client[OPEN_MAX_];//分配数组维护客户信息，-1表示所在项未使用，否则为描述符
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        err_sys("socket error");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(26001);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        err_sys("bind error");

    if (listen(listenfd, SOMAXCONN) < 0)
        err_sys("listen error");

    client[0].fd = listenfd;//client[0]用于固定监听套接字
    client[0].events = POLLRDNORM;//将第一项设置为POLLRDNORM事件
    for (i = 1; i < OPEN_MAX_; i++)
        client[i].fd = -1;        /* -1 indicates available entry */
    maxi = 0;                    /* max index into client[] array */

    for (;;) {
        nready = poll(client, maxi + 1, INFTIM);
        if (nready < 0)
            err_sys("poll error");

        if (client[0].revents & POLLRDNORM) /* new client connection */
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            if (connfd < 0)
                err_sys("accept error");

            printf("new client: %s, port %d\n", inet_ntoa(cliaddr.sin_addr),
                   ntohs(cliaddr.sin_port));

            for (i = 1; i < OPEN_MAX_; i++)
                if (client[i].fd < 0) {
                    client[i].fd = connfd;    /* save descriptor */
                    break;
                }
            if (i == OPEN_MAX_)
                err_quit("too many clients");

            client[i].events = POLLRDNORM;
            if (i > maxi)
                maxi = i;                /* max index in client[] array */

            if (--nready <= 0)
                continue;                /* no more readable descriptors */
        }

        for (i = 1; i <= maxi; i++) /* check all clients for data */
        {
            if ((sockfd = client[i].fd) < 0)
                continue;
            //检查返回事件POLLRDNORM和POLLERR
            //我们检查 POLLERR 的原因在于：有些实现在一个连接上接收到 RST 时返回的是 POLLERR 事件，
            //   而其他实现返回的只是 POLLRDNORM 事件。
            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        /*4connection reset by client */
                        printf("client[%d] aborted connection\n", i);

                        if (close(sockfd) == -1)
                            err_sys("close error");
                        client[i].fd = -1;
                    } else
                        err_sys("read error");
                } else if (n == 0) {
                    /*4connection closed by client */
                    printf("client[%d] closed connection\n", i);

                    if (close(sockfd))
                        err_sys("closer error");
                    client[i].fd = -1;
                } else {
                    if (writen(sockfd, buf, n) != n)
                        err_sys("writen error");
                }

                if (--nready <= 0)
                    break;                /* no more readable descriptors */
            }
        }
    }
}