// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "muduo/net/Buffer.h"

#include "muduo/net/SocketsOps.h"

#include <errno.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

/**
在非阻塞网络编程中，如何设计并使用缓冲区？一方面我们希望减少系统调用，一次读的数据越多越划算，那么似乎应该准备一个大的缓冲区。另一方面希望减少内存占
   用。如果有 10 000 个并发连接，每个连接一建立就分配各 50kB 的读写缓冲区的话，将占用 1GB 内存，而大多数时候这些缓冲区的使用率很低。muduo 用
   readv(2) 结合栈上空间巧妙地解决了这个问题。
具体做法是，在栈上准备一个 65 536 字节的 extrabuf，然后利用 readv() 来读取数据，iovec 有两块，第一块指向 muduoBuffer 中的 writable 字节，
   另一块指向栈上的 extrabuf。这样如果读入的数据不多，那么全部都读到 Buffer中去了；如果长度超过 Buffer 的 writable 字节数，就会读到栈上的
   extrabuf 里，然后程序再把 extrabuf 里的数据 append() 到 Buffer 中。
这么做利用了临时栈上空间【readFd() 是最内层函数，其在每个 IO 线程的最大 stack 空间开销是固定的 64KiB，与连接数目无关。如果 stack 空间紧张，
   也可以改用 thread local 的 extrabuf，但是不能全局共享一个 extrabuf。（为什么？）】，避免每个连接的初始 Buffer 过大造成的内存浪费，也避
   免反复调用 read() 的系统开销（由于缓冲区足够大，通常一次 readv() 系统调用就能读完全部数据）。由于 muduo 的事件触发采用 level trigger，
   因此这个函数并不会反复调用 read() 直到其返回 EAGAIN，从而可以降低消息处理的延迟。
这个实现有几点值得一提：
   一是使用了 scatter/gather IO，并且一部分缓冲区取自 stack，这样输入缓冲区足够大，通常一次 readv(2) 调用就能取完全部数据【在一个不繁忙（没
      有出现消息堆积）的系统上，程序一般等待在 poll(2) 上，一有数据到达就会立刻唤醒应用程序来读取，那么每次 read() 的数据不会超过几 KiB（一两个以
      太网 frame），这里 64KiB 缓冲足够容纳千兆网在 500μs 内全速收到的数据，在一定意义下可视为延迟带宽积（bandwidth-delay product）】。
      由于输入缓冲区足够大，也节省了一次 ioctl(socketFd, FIONREAD, &length) 系统调用，不必事先知道有多少数据可读而提前预留（reserve()）
      Buffer 的 capacity()，可以在一次读取之后将 extrabuf 中的数据 append() 给 Buffer。
   二是 Buffer::readFd() 只调用一次 read(2)，而没有反复调用 read(2) 直到其返回 EAGAIN。首先，这么做是正确的，因为 muduo 采用 level trigger，
      这么做不会丢失数据或消息。其次，对追求低延迟的程序来说，这么做是高效的，因为每次读数据只需要一次系统调用。再次，这样做照顾了多个连接的公平性，
      不会因为某个连接上数据量过大而影响其他连接处理消息。
 */

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable)
  {
    writerIndex_ += n;
  }
  else
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  // if (n == writable + sizeof extrabuf)
  // {
  //   goto line_30;
  // }
  return n;
}

