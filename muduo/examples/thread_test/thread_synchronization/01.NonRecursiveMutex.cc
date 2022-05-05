/// 存在问题：如果Foo::doit()间接调用了post()，那么会很有戏剧性的结果：
///         1. mutex是非递归的，于是死锁了。
///         2. mutex是递归的，由于push_back()可能（但不总是）导致vector迭代器失效，程序偶尔会crash。
///
/// 解决方法：如果确实需要在遍历的时候修改vector，有两种做法：
///     一是把修改推后，记住循环中试图添加或删除哪些元素，等循环结束了再依记录修改foos；
///     二是用copy-on-write

#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include <vector>
#include <stdio.h>

using namespace muduo;

class Foo {
public:
    void doit() const;
};

MutexLock mutex;
std::vector<Foo> foos;

void post(const Foo &f) {
    MutexLockGuard lock(mutex);
    foos.push_back(f);
}

void traverse() {
    MutexLockGuard lock(mutex);
    for (std::vector<Foo>::const_iterator it = foos.begin();
         it != foos.end(); ++it) {
        it->doit();
    }
}

void Foo::doit() const {
    Foo f;
    post(f); // 如果这里又调用了 post() 函数
}

int main() {
    Foo f;
    post(f);
    traverse();
}