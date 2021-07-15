#pragma once
#include <mutex>
#include <map>
#include <memory>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

// TimerCallback 必须保证并发安全
typedef void (*TimerCallback)();

// AsyncTimer 用来处理定时任务和延时任务
// 每个 AsyncTimer 对象含有一个处理线程，不同的任务串行处理
class AsyncTimer {
public:
    AsyncTimer(): _start(false) {}
    AsyncTimer(const AsyncTimer&) = delete;
    AsyncTimer &operator=(const AsyncTimer&) = delete;

    void start();
    void stop();
    bool isRun() const {return _start;}
    void addTimer(const std::string &name, int interval, TimerCallback callback);
    void delTimer(const std::string &name);
    void addAfter(const std::string &name, int wait, TimerCallback callback);
    void delAfter(const std::string &name);

private:
    static void loop(AsyncTimer *self);
    bool check(int64_t now);

private:
    struct WaitEvent;
    bool _start;
    std::mutex _mut;
    std::multimap<int64_t, std::string> _timers; // 时间戳->event name
    std::map<std::string, std::shared_ptr<WaitEvent>> _events; // event name ->
};

extern AsyncTimer *globalAsyncTimer();

UTILITY_NAMESPACE_END