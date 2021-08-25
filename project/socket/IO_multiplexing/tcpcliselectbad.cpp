#include    "base_api.h"

void str_cli(FILE *fp, int sockfd) {
    int maxfdp1;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);
    for (;;) {
        //一位对应于标准 I/O 文件指针 fp，一位对应于套接字 sockfd
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = std::max(fileno(fp), sockfd) + 1;
        if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
            err_sys("select error");

        if (FD_ISSET(sockfd, &rset)) {    /* socket is readable */
            if (Readline(sockfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");

            if (fputs(recvline, stdout) == EOF)
                err_sys("fputs error");
        }

        if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
            char *rptr;
            if ((rptr = fgets(sendline, MAXLINE, fp)) == NULL && ferror(fp))
                err_sys("fgets error");
                //问题1：当标准输入到达EOF时，则str_cli返回了，main函数也终止了；
                //  然而对于批量请求来说，标准输入中的EOF并不意味着我们同时也完成了从套接字的读入（可能仍有请求在
                //  去往服务器的路上，或者仍有应答在返回客户的路上）;
                //解决方法：使用shutdown进行半关闭
            else if (rptr == NULL)
                return;  /* all done */

            if (writen(sockfd, sendline, strlen(sendline)) != strlen(sendline))
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