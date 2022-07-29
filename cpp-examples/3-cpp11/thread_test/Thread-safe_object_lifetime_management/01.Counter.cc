#include "muduo/base/Mutex.h"

using muduo::MutexLock;
using muduo::MutexLockGuard;

/// 问题：单线程安全的 Counter 示例，多线程时是有问题的
// A thread-safe counter
class Counter : muduo::noncopyable {
    // copy-ctor and assignment should be private by default for a class.
public:
    Counter() : value_(0) {}

    Counter &operator=(const Counter &rhs);

    int64_t value() const;

    int64_t getAndIncrease();

    friend void swap(Counter &a, Counter &b);

private:
    mutable MutexLock mutex_;
    int64_t value_;
};

int64_t Counter::value() const {
    MutexLockGuard lock(mutex_); // lock 的析构会晚于返回对象的构造，v
    return value_; // 因此有效地保护了这个共享数据。
}

int64_t Counter::getAndIncrease() {
    MutexLockGuard lock(mutex_);
    int64_t ret = value_++;
    return ret;
}

/// 可能存在死锁的情况
///
/// 如果线程A执行swap(a, b)；而同时线程B执行swap(b,a)；就有可能死锁。
void swap(Counter &a, Counter &b) {
    MutexLockGuard aLock(a.mutex_);  // potential dead lock
    MutexLockGuard bLock(b.mutex_);
    int64_t value = a.value_;
    a.value_ = b.value_;
    b.value_ = value;
}

Counter &Counter::operator=(const Counter &rhs) {
    if (this == &rhs)
        return *this;

    MutexLockGuard myLock(mutex_);  // potential dead lock
    MutexLockGuard itsLock(rhs.mutex_);
    value_ = rhs.value_;
    return *this;
}

int main() {
    Counter c;
    c.getAndIncrease();
}