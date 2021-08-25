#include    "base_api.h"

void str_cli(FILE *fp, int sockfd) {
    int maxfdp1, stdineof;
    fd_set rset;
    char buf[MAXLINE];
    int n;

    stdineof = 0;
    FD_ZERO(&rset);
    for (;;) {
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = std::max(fileno(fp), sockfd) + 1;
        if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
            err_sys("select error");

        if (FD_ISSET(sockfd, &rset)) /* socket is readable */
        {
            if ((n = read(sockfd, buf, MAXLINE)) < 0)
                err_sys("read error");
            else if (n == 0) {
                if (stdineof == 1)
                    return;        /* normal termination */
                else
                    err_quit("str_cli: server terminated prematurely");
            }

            if (write(fileno(stdout), buf, n) != n)
                err_sys("write error");
        }

        if (FD_ISSET(fileno(fp), &rset)) /* input is readable */
        {
            if ((n = read(fileno(fp), buf, MAXLINE)) < 0)
                err_sys("read error");
            else if (n == 0) {
                stdineof = 1;//当在标准输入上碰到EOF，则将stdineof置为1，并调用shutdown
                if (shutdown(sockfd, SHUT_WR) < 0)    /* send FIN */
                    err_sys("shutdown error");
                FD_CLR(fileno(fp), &rset);
                continue;
            }

            if (writen(sockfd, buf, n) != n)
                err_sys("writen error");
        }
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("usage: tcpcli <IPaddress>\n");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        err_sys("socket error");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(26001);

    int ret = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    if (ret < 0)
        err_sys("inet_pton error");      /* errno set */
    else if (ret == 0)
        err_quit("inet_pton error:invalue IP format");     /* errno not set */

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        err_sys("connect error");

    str_cli(stdin, sockfd);        /* do it all */

    exit(0);
}