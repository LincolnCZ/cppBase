// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
/**每个 Channel 对象自始至终只属于一个 EventLoop，因此每个 Channel 对象都只属于某一个 IO 线程。每个 Channel 对象自始至终只负责一个文件
 * 描述符（fd）的 IO 事件分发，但它并不拥有这个 fd，也不会在析构的时候关闭这个 fd。Channel 会把不同的 IO 事件分发为不同的回调。
 * Channel 的生命期由其 owner class 负责管理，它一般是其他 class 的直接或间接成员。
 * */
class Channel : boost::noncopyable
{
 public:
  typedef boost::function<void()> EventCallback;

  Channel(EventLoop* loop, int fd);

  void handleEvent();
  void setReadCallback(const EventCallback& cb)
  { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() { events_ |= kReadEvent; update(); }
  // void enableWriting() { events_ |= kWriteEvent; update(); }
  // void disableWriting() { events_ &= ~kWriteEvent; update(); }
  // void disableAll() { events_ = kNoneEvent; update(); }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

 private:
  void update();

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int  fd_;
  int        events_;
  int        revents_;
  int        index_; // used by Poller.

  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
};

}
#endif  // MUDUO_NET_CHANNEL_H
