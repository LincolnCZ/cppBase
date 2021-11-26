#include "muduo/base/Mutex.h"

class Request {
public:
    void process() // __attribute__ ((noinline))
    {
        muduo::MutexLockGuard lock(mutex_);
        // ...
        print(); // 原本没有这行，某人为了调试程序不小心添加
    }

    void print() const // __attribute__ ((noinline))
    {
        muduo::MutexLockGuard lock(mutex_);
    }

private:
    mutable muduo::MutexLock mutex_;
};

int main() {
    Request req;
    req.process();
}