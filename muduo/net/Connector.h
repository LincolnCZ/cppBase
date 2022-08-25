// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_CONNECTOR_H
#define MUDUO_NET_CONNECTOR_H

#include "muduo/base/noncopyable.h"
#include "muduo/net/InetAddress.h"

#include <functional>
#include <memory>

namespace muduo
{
namespace net
{

class Channel;
class EventLoop;

/**
Connector 用于发起 TCP 连接，它是 TcpClient 的成员，生命期由后者控制。
* 在非阻塞网络编程中，发起连接的基本方式是调用 connect(2)，当 socket 变得可写时表明连接建立完毕。
* Connector 只负责建立 socket 连接，不负责创建 TcpConnection，它的 NewConnectionCallback 回调的参数是 socket 文件描述符。

 Connector 的实现有几个难点：
• socket 是一次性的，一旦出错（比如对方拒绝连接），就无法恢复，只能关闭重来。但 Connector 是可以反复使用的，因此每次尝试连接都要使用新的
  socket 文件描述符和新的 Channel 对象。要留意 Channel 对象的生命期管理，并防止 socket 文件描述符泄漏。
• 错误代码与 accept(2) 不同，EAGAIN 是真的错误，表明本机 ephemeral port 暂时用完，要关闭 socket 再延期重试。“正在连接”的返回码是
  EINPROGRESS。另外，即便出现 socket 可写，也不一定意味着连接已成功建立，还需要用 getsockopt(sockfd, SOL_SOCKET, SO_ERROR, ...) 再次确认一下。
• 重试的间隔应该逐渐延长，例如 0.5s、1s、2s、4s，直至 30s，即 back-off。这会造成对象生命期管理方面的困难，如果使用EventLoop::runAfter()
  定时而 Connector 在定时器到期之前析构了怎么办？
• 要处理自连接（self-connection）。出现这种状况的原因如下。在发起连接的时候，TCP/IP 协议栈会先选择 source IP 和 source port，在没有显式调用
  bind(2) 的情况下，sourceIP由 路由表确定，source port 由 TCP/IP 协议栈从 localport range 中选取尚未使用的 port（即 ephemeralport ）。
  如果 destination IP 正好是本机，而 destination port 位于 local port range，且没有服务程序监听的话，ephemeral port 可能正好选中了
  destination port，这就出现( source IP,  source port) = (destination IP, destination port) 的情况，即发生了自连接。处理办法是断
  开连接再重试，否则原本侦听 destination port 的服务进程也无法启动了。
*/
class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
 public:
  typedef std::function<void (int sockfd)> NewConnectionCallback;

  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }

  void start();  // can be called in any thread
  void restart();  // must be called in loop thread
  void stop();  // can be called in any thread

  const InetAddress& serverAddress() const { return serverAddr_; }

 private:
  enum States { kDisconnected, kConnecting, kConnected };
  static const int kMaxRetryDelayMs = 30*1000;
  static const int kInitRetryDelayMs = 500;

  void setState(States s) { state_ = s; }
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop* loop_;
  InetAddress serverAddr_;
  bool connect_; // atomic
  States state_;  // FIXME: use atomic variable
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback newConnectionCallback_;
  int retryDelayMs_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_CONNECTOR_H
