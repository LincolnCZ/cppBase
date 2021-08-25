#include    "base_api.h"

int main(int argc, char **argv) {
    //变量 maxi 是 client 数组当前使用项的最大下标，
    //而变量 maxfd（加 1 之后）是 select 函数第一个参数的当前值
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
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

    maxfd = listenfd;            /* initialize */
    maxi = -1;                    /* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;            /* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;        /* structure assignment */
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0)
            err_sys("select error");

        //select 等待某个事件发生：或是新客户连接的建立，或是数据、FIN 或 RST 的到达

        if (FD_ISSET(listenfd, &rset)) /* new client connection */
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            if (connfd < 0)
                err_sys("accept error");

            printf("new client: %s, port %d\n", inet_ntoa(cliaddr.sin_addr),
                   ntohs(cliaddr.sin_port));

            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd;    /* save descriptor */
                    break;
                }
            if (i == FD_SETSIZE)
                err_quit("too many clients");

            FD_SET(connfd, &allset);    /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;            /* for select */
            if (i > maxi)
                maxi = i;                /* max index in client[] array */

            if (--nready <= 0)
                continue;                /* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) /* check all clients for data */
        {
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = read(sockfd, buf, MAXLINE)) < 0)
                    err_sys("read error");
                else if (n == 0)//如果客户关闭了连接，那么read返回0
                {
                    /*connection closed by client */
                    if (close(sockfd) == -1)
                        err_sys("close error");

                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
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