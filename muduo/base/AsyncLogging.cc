// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/AsyncLogging.h"
#include "muduo/base/LogFile.h"
#include "muduo/base/Timestamp.h"

#include <stdio.h>

using namespace muduo;

AsyncLogging::AsyncLogging(const string& basename,
                           off_t rollSize,
                           int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

/*发送方实现
 * 操作（1）（2）两种情况在临界区之内都没有耗时的操作，运行时间为常数。
 * */
void AsyncLogging::append(const char* logline, int len)
{
  muduo::MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len)
  {
    /*（1）如果当前缓冲（currentBuffer_）剩余的空间足够大，则会直接把日志消息拷贝（追加）到当前缓冲中，*/
    currentBuffer_->append(logline, len);
  }
  else
  {
    buffers_.push_back(std::move(currentBuffer_));

    if (nextBuffer_)
    {
      /*（2）把预备好的另一块缓冲（nextBuffer_）移用（move）为当前缓冲，然后追加日志消息并通知（唤醒）后端开始写入日志数据*/
      currentBuffer_ = std::move(nextBuffer_); // 移动而非复制
    }
    else
    {
      /*（3）如果前端写入速度太快，一下子把两块缓冲都用完了，那么只好分配一块新的buffer，作为当前缓冲，这是极少发生的情况。*/
      currentBuffer_.reset(new Buffer); // Rarely happens
    }
    currentBuffer_->append(logline, len);
    cond_.notify();
  }
}

/*接收方（后端）实现
 * 注意到后端临界区内也没有耗时的操作，运行时间为常数。
 * */
void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();
  LogFile output(basename_, rollSize_, false);
  /*先准备好两块空闲的buffer，以备在临界区内交换*/
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_)
  {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      muduo::MutexLockGuard lock(mutex_);
      /*在临界区内，等待条件触发，这里的条件有两个：其一是超时，其二是前端写满了一个或多个buffer。
       * 注意这里是非常规的condition variable用法，它没有使用while循环，而且等待时间有上限。
       */
      if (buffers_.empty())  // unusual usage!
      {
        cond_.waitForSeconds(flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_)); // 移动，而非复制
      currentBuffer_ = std::move(newBuffer1); // 移动，而非复制
      buffersToWrite.swap(buffers_); // 内部指针交换，而非复制

      /*用newBuffer2替换nextBuffer_，这样前端始终有一个预备buffer可供调配。
       * nextBuffer_可以减少前端临界区分配内存的概率，缩短前端临界区长度。
       */
      if (!nextBuffer_)
      {
        nextBuffer_ = std::move(newBuffer2); // 移动，而非复制
      }
    }

    /*以在临界区之外安全地访问buffersToWrite，将其中的日志数据写入文件*/
    assert(!buffersToWrite.empty());
    if (buffersToWrite.size() > 25)
    {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size()-2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite)
    {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2)
    {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    /*将buffersToWrite内的buffer重新填充newBuffer1和newBuffer2，这样下一次执行的时候还有两个空闲buffer可用于替换前端的当前缓冲和预备缓冲。*/
    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }
    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}

