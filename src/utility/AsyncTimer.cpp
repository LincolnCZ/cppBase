#include <thread>
#include <vector>
#include <unistd.h>
#include "utility/AsyncTimer.h"

UTILITY_NAMESPACE_BEGIN

struct AsyncTimer::WaitEvent {
    bool isTimer;
    int interval;
    TimerCallback callback;
};

void AsyncTimer::start() {
    _start = true;
    std::thread th(loop, this);
    th.detach();
}

void AsyncTimer::stop() {
    _mut.lock();
    _start = false;
    _mut.unlock();
}

void AsyncTimer::addTimer(const std::string &name, int interval, TimerCallback callback) {
    int64_t now = time(NULL);
    _mut.lock();
    auto ev = std::make_shared<WaitEvent>();
    ev->isTimer = true;
    ev->interval = interval;
    ev->callback = callback;
    _events[name] = ev;
    _timers.insert(std::make_pair(now + interval, name));
    _mut.unlock();
}

void AsyncTimer::delTimer(const std::string &name) {
    _mut.lock();
    _events.erase(name);
    _mut.unlock();
}

void AsyncTimer::addAfter(const std::string &name, int wait, TimerCallback callback) {
    int64_t now = time(NULL);

    _mut.lock();
    auto ev = std::make_shared<WaitEvent>();
    ev->isTimer = false;
    ev->interval = 0;
    ev->callback = callback;
    _events[name] = ev;
    _timers.insert(std::make_pair(now + wait, name));
    _mut.unlock();
}

void AsyncTimer::delAfter(const std::string &name) {
    _mut.lock();
    _events.erase(name);
    _mut.unlock();
}

void AsyncTimer::loop(AsyncTimer *self) {
    while (true) {
        struct timespec st;
        clock_gettime(CLOCK_REALTIME, &st);
        int64_t now = st.tv_sec;

        if (!self->check(now))
            break;

        struct timespec ed;
        clock_gettime(CLOCK_REALTIME, &ed);
        if (ed.tv_sec == st.tv_sec)
            usleep(1000000 - st.tv_nsec / 1000);
    }
}

bool AsyncTimer::check(int64_t now) {
    std::vector<std::shared_ptr<WaitEvent>> events;

    _mut.lock();
    if (_start == false) {
        _mut.unlock();
        return false;
    }

    while (true) {
        auto it = _timers.begin();
        if (it == _timers.end() || it->first > now)
            break;
        std::string name = it->second;
        _timers.erase(it);

        auto itev = _events.find(name);
        if (itev != _events.end()) {
            std::shared_ptr<WaitEvent> ev = itev->second;
            if (ev->isTimer)
                _timers.insert(std::make_pair(now + ev->interval, name));
            else
                _events.erase(itev);
            events.push_back(std::move(ev));
        }
    }
    _mut.unlock();

    for (size_t i = 0; i < events.size(); i++) {
        events[i]->callback();
    }
    return true;
}

static AsyncTimer *globalTimer;
static std::once_flag timerOnce;

AsyncTimer *globalAsyncTimer() {
    if (globalTimer == NULL) {
        std::call_once(timerOnce, [] {
            globalTimer = new AsyncTimer;
            globalTimer->start();
        });
    }
    return globalTimer;
}

UTILITY_NAMESPACE_END