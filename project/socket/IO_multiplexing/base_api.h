#ifndef BASE_API_H
#define BASE_API_H

#include <string>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAXLINE     4096        /* max text line length */

typedef void Sigfunc(int);

inline void err_sys(const std::string &msg) {
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}

inline void err_quit(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}


Sigfunc *signal(int signo, Sigfunc *func) {
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo == SIGALRM) {
        //对 SIGALRM 进行特殊处理的原因在于：产生该信号的目的通常为 I/O 操作设置超时，
        //这种况下我们希望受阻塞的系统调用被该信号中断掉。
#ifdef  SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;   /* SunOS 4.x */
#endif
    } else {
        //设置SA_RESTART标志，则由相应信号中断的系统调用将由内核自动重启
#ifdef  SA_RESTART
        act.sa_flags |= SA_RESTART;             /* SVR4, 44BSD */
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

Sigfunc *Signal(int signo, Sigfunc *func)        /* for our signal() function */
{
    Sigfunc *sigfunc;

    if ((sigfunc = signal(signo, func)) == SIG_ERR)
        err_sys("signal error");
    return (sigfunc);
}


//***************************************
//返回值说明:
//    == n: 说明正确返回, 已经真正写入了n个字节
//    == -1: 写入出错返回
//***************************************
ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (char *) vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            //如果写入操作被信号中断，则应该继续进行写操作
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
                //发生其他错误
            else
                return (-1);                     /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

void Writen(int fd, void *ptr, size_t nbytes) {
    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen error");
}

//***************************************
//返回值说明:
//    == n: 说明正确返回, 已经真正读取了n个字节
//    == -1: 读取出错返回
//    0<= return < n:EOF 到达了文件尾
//***************************************
ssize_t readn(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char *) vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;//如果读取操作被信号中断，则应该继续进行读操作
            else
                return (-1);//其他错误
        } else if (nread == 0)
            break;                /* EOF 到达了文件尾*/

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);        /* return >= 0 */
}

ssize_t Readn(int fd, void *ptr, size_t nbytes) {
    ssize_t n;

    if ((n = readn(fd, ptr, nbytes)) < 0)
        err_sys("readn error");
    return (n);
}

//*********************************************************
//在 readline 的实现中使用静态变量实现跨相继函数调用的状态信息维护， 
//其结果是这些函数变得不可重入或者说非线程安全了。
//*********************************************************
static int read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];

static ssize_t my_read(int fd, char *ptr) {
    if (read_cnt <= 0) {
        again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return (-1);
        } else if (read_cnt == 0)
            return (0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return (1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = (char *) vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;    /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);    /* EOF, n - 1 bytes were read */
        } else
            return (-1);        /* error, errno set by read() */
    }

    *ptr = 0;    /* null terminate like fgets() */
    return (n);
}

ssize_t readlinebuf(void **vptrptr) {
    if (read_cnt)
        *vptrptr = read_ptr;
    return (read_cnt);
}

/* end readline */

ssize_t Readline(int fd, void *ptr, size_t maxlen) {
    ssize_t n;

    if ((n = readline(fd, ptr, maxlen)) < 0)
        err_sys("readline error");
    return (n);
}

#endif 