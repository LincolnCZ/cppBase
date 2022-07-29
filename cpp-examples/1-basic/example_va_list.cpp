#include <cstdio>
#include <cstdarg>
#include <cstring>

/**怎样写出一个可以处理像 printf 一样能够处理可变长参数的函数呢。
看printf的定义:

int printf(char *fmt， ...)；

C语言标准库中头文件 stdarg.h 索引的接口包含了一组能够遍历变长参数列表的宏。主要包含下面几个：

1、va_list 用来声明一个表示参数表中各个参数的变量。
2、va_start 初始化一个指针来指向变长参数列表的头一个变量（注意，...只能出现在参数表的最后）
3、va_arg 每次调用时都会返回当前指针指向的变量，并将指针挪至下一个位置，
    参数的类型需要在这个调用的第二个参数来指定，va_arg 也是根据这个参数来判断偏移的距离。
4、va_end 需要在函数最后调用，来进行一些清理工作。**/

void write_log(char *fmt, ...) {
    va_list va;
    char buf[1024];

    va_start(va, fmt);
    memset(buf, 0, 1024);
    (void) vsprintf(buf, fmt, va);
    va_end(va);

    printf("%s-%s", "my_log_pre_head", buf);
}

void read_num(int num, ...) {
    va_list va;             /*point to each unnamed variables in arg list*/
    va_start(va,num);      /*start va_list from num, and va goes to the second one, and this is the first vary variable*/
    while (num--) {
        printf("%d\t", va_arg(va, int)); /*get a arg, va goes to the next*/
    }
    va_end(va);             /*end the va*/
}

int main() {
    write_log("%s\n", "hello world!");
    read_num(3, 111, 222, 333);
    return 0;
}
// 输出结果：
// my_log_pre_head-hello world!
// 111	222	333
