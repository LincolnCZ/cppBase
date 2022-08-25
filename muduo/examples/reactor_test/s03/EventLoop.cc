// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "EventLoop.h"

#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include "muduo/base/Logging.h"

#include <boost/bind.hpp>

#include <assert.h>
#include <sys/eventfd.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(new Poller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread)
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(
      boost::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
  assert(!looping_);
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_)
  {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
        it != activeChannels_.end(); ++it)
    {
      (*it)->handleEvent();
    }
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  if (!isInLoopThread())
  {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor& cb)
{
  if (isInLoopThread())
  {
    cb();
  }
  else
  {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor& cb)
{
  {
  MutexLockGuard lock(mutex_);
  pendingFunctors_.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

/**
 *（1）EventLoop::doPendingFunctors() 不是简单地在临界区内依次调用 Functor，而是把回调列表 swap() 到局部变量 functors 中，
 *  这样一方面减小了临界区的长度（意味着不会阻塞其他线程调用 queueInLoop() ），另一方面也避免了死锁（因为 Functor 可能再调用 queueInLoop() ）。
 *（2）由于 doPendingFunctors() 调用的 Functor 可能再调用 queueInLoop(cb)，这时 queueInLoop() 就必须 wakeup()，否则这些新加的 cb 就不能
 *  被及时调用了。muduo 这里没有反复执行 doPendingFunctors() 直到 pendingFunctors_ 为空，这是有意的，否则 IO 线程有可能陷入死循环，无法处理 IO 事件。
 *（3）注意 muduo 不是在EventLoop::handleRead() 中执行 doPendingFunctors()
 *  一个是节省系统调用，试想一下如果 doPendingFunctors() 放到 EventLoop::handleRead() 中，就必须调用 weakup()，这样流程就涉及三个
 *  系统调用 write->poll->read。在 IO 线程内注册回调函数这种情况下，其用不着花费三个系统。因为注册回调函数这个动作本身也是被回调的，也就是
 *  说 EventLoop::loop() 里面循环调用完 handleEvent（）后（这时注册回调函数这个动作已完成），就可以立马调用doPendingFunctors（）去处
 *  理回调了，省略三次系统调用时间；另一个是我觉得 pendingFunctors 优先级低于其他事件。
 */
void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
  MutexLockGuard lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i)
  {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}

