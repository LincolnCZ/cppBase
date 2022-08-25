// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Acceptor.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(
      std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

void Acceptor::listen()
{
  loop_->assertInLoopThread();
  listenning_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0)
  {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    /**这里直接把 socket fd 传给 callback，这种传递 int 句柄的做法不够理想，在 C++11 中可以先创建 Socket 对象，再用移动语义
     *   把 Socket 对象 std::move() 给回调函数，确保资源的安全释放。*/
    if (newConnectionCallback_)
    {
      newConnectionCallback_(connfd, peerAddr);
    }
    else
    {
      sockets::close(connfd);
    }
  }
  else
  {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE)
    {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

/**限制服务器的最大并发连接数*/
//在服务端网络编程中，我们通常用Reactor模式来处理并发连接。listening socket是一种特殊的IO对象，当有新连接到达时，此listening文件描述符变
//   得可读（POLLIN），epoll_wait返回这一事件。然后我们用accept(2)系统调用获得新连接的socket文件描述符。代码主体逻辑如下（Python）：
//
//#!/usr/bin/python
//
//import socket
//import select
//server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
//server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
//server_socket.bind(('', 2007))
//server_socket.listen(5)
//# server_socket.setblocking(0)
//poll = select.poll() # epoll() should work the same
//poll.register(server_socket.fileno(), select.POLLIN)
//
//connections = {}
//while True:
//    events = poll.poll(10000)  # 10 seconds
//    for fileno, event in events:
//        if fileno == server_socket.fileno():
//            (client_socket, client_address) = server_socket.accept()
//            print "got connection from", client_address
//            # client_socket.setblocking(0)
//            poll.register(client_socket.fileno(), select.POLLIN)
//            connections[client_socket.fileno()] = client_socket
//        elif event & select.POLLIN:
//            client_socket = connections[fileno]
//            data = client_socket.recv(4096)
//            if data:
//                client_socket.send(data) # sendall() partial?
//            else:
//                poll.unregister(fileno)
//                client_socket.close()
//                del connections[fileno]
//
//假如 accept(2) 返回 EMFILE 该如何应对？这意味着本进程的文件描述符已经达到上限，无法为新连接创建 socket 文件描述符。但是，既然没有 socket 文件
//   描述符来表示这个连接，我们就无法 close(2) 它。程序继续运行，再一次调用 epoll_wait。这时候 epoll_wait 会立刻返回，因为新连接还等待处理，
//   listening fd 还是可读的。这样程序立刻就陷入了 busyloop，CPU 占用率接近 100%.这既影响同一 event loop 上的连接，也影响同一机器上的其他服务。
//
// 该怎么办呢？Marc Lehmann 提到了几种做法：
//1. 调高进程的文件描述符数目。治标不治本，因为只要有足够多的客户端，就一定能把一个服务进程的文件描述符用完。
//2. 死等。鸵鸟算法。
//3. 退出程序。似乎小题大做，为了这种暂时的错误而中断现有的服务似乎不值得。
//4. 关闭listening fd。那么什么时候重新打开呢？
//5. 改用edge trigger。如果漏掉了一次accept(2)，程序再也不会收到新连接。
//6. 准备一个空闲的文件描述符。遇到这种情况，先关闭这个空闲文件，获得一个文件描述符的名额；再accept(2)拿到新socket连接的描述符；随后立刻
//     close(2)它，这样就优雅地断开了客户端连接；最后重新打开一个空闲文件，把“坑”占住，以备再次出现这种情况时使用。
//
//第2、5两种做法会导致客户端认为连接已建立，但无法获得服务，因为服务端程序没有拿到连接的文件描述符。
//muduo 的 Acceptor 正是用第 6 种方案实现的。但是，这个做法在多线程下不能保证正确，会有race condition。（思考题：是什么race condition？）
//其实有另外一种比较简单的办法：file descriptor 是 hardlimit，我们可以自己设一个稍低一点的 soft limit，如果超过 softlimit 就主动关闭新连接，
//   这样就可避免触及 “file descriptor耗尽” 这种边界条件。比方说当前进程的 max file descriptor 是 1024，那么我们可以在连接数达到 1000 的时候进入
//   “拒绝新连接”状态，这样就可留给我们足够的腾挪空间。