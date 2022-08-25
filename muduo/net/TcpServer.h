// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include "muduo/base/Atomic.h"
#include "muduo/base/Types.h"
#include "muduo/net/TcpConnection.h"

#include <map>

namespace muduo
{
namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
/**TcpServer 用于编写网络服务器，接受客户的连接。
 * TcpServer class 的功能是管理accept(2)获得的 TcpConnection。TcpServer 是供用户直接使用的，生命期由用户控制。
 * TcpServer 内部使用 Acceptor 来获得新连接的 fd。它保存用户提供的 ConnectionCallback 和 MessageCallback，在新建 TcpConnection 的时候会
 *   原样传给后者。TcpServer 持有目前存活的 TcpConnection 的 shared_ptr（定义为TcpConnectionPtr），因为 TcpConnection 对象的生命期是模糊的，
 *   用户也可以持有 TcpConnectionPtr。
 *
 * 多线程 TcpServer：
 *   用 one loop per thread 的思想实现多线程 TcpServer 的关键步骤是在新建 TcpConnection 时从 event loop pool 里挑选一个 loop 给
 *      TcpConnection 用。也就是说多线程 TcpServer 自己的 EventLoop 只用来接受新连接，而新连接会用其他 EventLoop 来执行 IO。（单线程
 *      TcpServer 的 EventLoop 是与 TcpConnection 共享的。）muduo 的 event loop pool 由 EventLoopThreadPoolclass 表示。
 *   总而言之，TcpServer 和 TcpConnection 的代码都只处理单线程的情况（甚至都没有 mutex 成员），而我们借助EventLoop::runInLoop() 并引入
 *      EventLoopThreadPool 让多线程 TcpServer 的实现易如反掌。
 *   */
class TcpServer : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  enum Option
  {
    kNoReusePort,
    kReusePort,
  };

  //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
  ~TcpServer();  // force out-line dtor, for std::unique_ptr members.

  const string& ipPort() const { return ipPort_; }
  const string& name() const { return name_; }
  EventLoop* getLoop() const { return loop_; }

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  /// valid after calling start()
  std::shared_ptr<EventLoopThreadPool> threadPool()
  { return threadPool_; }

  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  /// Thread safe.
  void removeConnection(const TcpConnectionPtr& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  typedef std::map<string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;  // the acceptor loop
  const string ipPort_;
  const string name_;
  std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  AtomicInt32 started_;
  // always in loop thread
  int nextConnId_;
  ConnectionMap connections_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TCPSERVER_H
