// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
/**
TimerQueue 用 timerfd 实现定时，这有别于传统的设置 poll/epoll_wait 的等待时长的办法。TimerQueue 用std::map来管理 Timer，
  常用操作的复杂度是 O(logN)，N 为定时器数目。它是 EventLoop 的成员，生命期由后者控制。
  • 注意 TimerQueue 的成员函数只能在其所属的IO线程调用，因此不必加锁。
 */
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  /**借助 EventLoop::runInLoop()，我们可以很容易地将 TimerQueue::addTimer() 做成线程安全的，而且无须用锁。
   *   办法是让 addTimer() 调用 runInLoop()，把实际工作转移到 IO 线程来做。*/
  TimerId addTimer(TimerCallback cb,
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer*, int64_t> ActiveTimer; /**<Timer*, sequence>*/
  typedef std::set<ActiveTimer> ActiveTimerSet;

  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;

  // for cancel()
  /**
   * cancel 实现采用更传统的方式，保持现有的设计，让 TimerId 包含 Timer*。但这是不够的，因为无法区分地址相同的先后两个 Timer 对象。
   *   因此每个 Timer 对象有一个全局递增的序列号 int64_t sequence_（用原子计数器（AtomicInt64）生成），TimerId 同时保存 Timer*
   *   和 sequence_，这样 TimerQueue::cancel() 就能根据 TimerId 找到需要注销的 Timer 对象。
   *
   * activeTimers_ 保存的是目前有效的 Timer 的指针，并满足 timers_.size()==activeTimers_.size()，因为这两个容器保存的是相同的数据，
   *   只不过 timers_ 是按到期时间排序，activeTimers_ 是按对象地址排序。
   * */
  ActiveTimerSet activeTimers_;
  /**cancelingTimers_ 和 callingExpiredTimers_ 是为了应对“自注销”这种情况，即在定时器回调中注销当前定时器。
   * 为了应对这种情况，TimerQueue 会记住在本次调用到期 Timer 期间有哪些 cancel() 请求，并且不再把已 cancel() 的 Timer 添加回 timers_
   *    和 activeTimers_ 当中。*/
  bool callingExpiredTimers_; /* atomic */
  ActiveTimerSet cancelingTimers_;
};

}  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_TIMERQUEUE_H
