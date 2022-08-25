// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPCLIENT_H
#define MUDUO_NET_TCPCLIENT_H

#include "muduo/base/Mutex.h"
#include "muduo/net/TcpConnection.h"

namespace muduo
{
namespace net
{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

/**TcpClient 用于编写网络客户端，能发起连接，并且有重试功能。
• TcpClient 具备 TcpConnection 断开之后重新连接的功能，加上 Connector 具备反复尝试连接的功能，因此客户端和服务端的启动顺序无关紧要。
  可以先启动客户端，一旦服务端启动，半分钟之内即可恢复连接（由 Connector::kMaxRetryDelayMs 常数控制）；在客户端运行期间服务端可以重启，客户端也会自动重连。
• 连接断开后初次重试的延迟应该有随机性，比方说服务端崩溃，它所有的客户连接同时断开，然后 0.5s 之后同时再次发起连接，这样既可能造成 SYN 丢包，
  也可能给服务端带来短期大负载，影响其服务质量。因此每个 TcpClient 应该等待一段随机的时间（0.5~2s），再重试，避免拥塞。
• 发起连接的时候如果发生 TCP SYN 丢包，那么系统默认的重试间隔是 3s，这期间不会返回错误码，而且这个间隔似乎不容易修改。如果需要缩短间隔，
  可以再用一个定时器，在 0.5s 或 1s 之后发起另一次连接。如果有需求的话，这个功能可以做到 Connector 中。
• 目前本节实现的 TcpClient 没有充分测试动态增减的情况，也就是说没有充分测试 TcpClient 的生命期比 EventLoop 短的情况，特别是没有充分测试
  TcpClient 在连接建立期间析构的情况。编写这方面的单元测试多半要用到 §12.4 介绍的技术。
 * */
class TcpClient : noncopyable
{
 public:
  // TcpClient(EventLoop* loop);
  // TcpClient(EventLoop* loop, const string& host, uint16_t port);
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const string& nameArg);
  ~TcpClient();  // force out-line dtor, for std::unique_ptr members.

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const
  {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }

  const string& name() const
  { return name_; }

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(ConnectionCallback cb)
  { connectionCallback_ = std::move(cb); }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = std::move(cb); }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(WriteCompleteCallback cb)
  { writeCompleteCallback_ = std::move(cb); }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd);
  /// Not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_; // avoid revealing Connector
  const string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;   // atomic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;
  mutable MutexLock mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TCPCLIENT_H
