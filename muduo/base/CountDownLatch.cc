// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/CountDownLatch.h"

using namespace muduo;

CountDownLatch::CountDownLatch(int count)
  : mutex_(),
    condition_(mutex_),
    count_(count)
{
}

void CountDownLatch::wait()
{
  MutexLockGuard lock(mutex_);
  while (count_ > 0)
  {
    condition_.wait();
  }
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0)
  {
    condition_.notifyAll();
    /**注意到CountDownLatch::countDown()使用的是Condition::notifyAll()，而BlockingQueue.h使用的是Condition::notify()，这都是有意为之。
     * 请读者思考，如果交换两种用法会出现什么情况？
     * 对于coundown_latch，因为计数器为0的情况只有一次，所以必须唤醒所有的等待线程，但是对于blockingqueue, 我觉得没有使用notifyAll的必要，
     * 唤醒没有资源还是要继续wait。*/
  }
}

int CountDownLatch::getCount() const
{
  MutexLockGuard lock(mutex_);
  return count_;
}

