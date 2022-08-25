// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_BUFFER_H
#define MUDUO_NET_BUFFER_H

#include "muduo/base/copyable.h"
#include "muduo/base/StringPiece.h"
#include "muduo/base/Types.h"

#include "muduo/net/Endian.h"

#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>
//#include <unistd.h>  // ssize_t

/**为什么 non-blocking 网络编程中应用层 buffer 是必需的*/
/*
non-blocking IO 的核心思想是避免阻塞在 read() 或 write() 或其他 IO 系统调用上，这样可以最大限度地复用 thread-of-control，让一个线程能
    服务于多个 socket 连接。IO 线程只能阻塞在 IO multiplexing 函数上，如select/poll/epoll_wait。这样一来，应用层的缓冲是必需的，每个
    TCP socket 都要有 stateful 的input buffer和output buffer。
（1）TcpConnection 必须要有 output buffer　
   考虑一个常见场景：程序想通过 TCP 连接发送 100kB 的数据，但是在 write() 调用中，操作系统只接受了 80kB（受 TCP advertised window 的
      控制，细节见[TCPv1]），你肯定不想在原地等待，因为不知道会等多久（取决于对方什么时候接收数据，然后滑动 TCP 窗口）。程序应该尽快交出控制权，
      返回 event loop。在这种情况下，剩余的 20kB 数据怎么办？
   对于应用程序而言，它只管生成数据，它不应该关心到底数据是一次性发送还是分成几次发送，这些应该由网络库来操心，程序只要调用 TcpConnection::send()
      就行了，网络库会负责到底。网络库应该接管这剩余的 20kB 数据，把它保存在该 TCPconnection 的 output buffer 里，然后注册 POLLOUT 事件，
      一旦 socket 变得可写就立刻发送数据。当然，这第二次 write() 也不一定能完全写入 20kB，如果还有剩余，网络库应该继续关注 POLLOUT 事件；如果写
      完了 20kB，网络库应该停止关注 POLLOUT，以免造成 busy loop。（muduoEventLoop 采用的是 epoll leveltrigger，原因见下页。）
   如果程序又写入了 50kB，而这时候 output buffer 里还有待发送的 20kB 数据，那么网络库不应该直接调用 write()，而应该把这 50kB 数据 append
      在那 20kB 数据之后，等 socket 变得可写的时候再一并写入。
   如果 output buffer 里还有待发送的数据，而程序又想关闭连接（对程序而言，调用TcpConnection::send()之后他就认为数据迟早会发出去），那么这时
      候网络库不能立刻关闭连接，而要等数据发送完毕，见§7.2“为什么 TcpConnection:: shutdown() 没有直接关闭 TCP 连接”中的讲解。
   综上，要让程序在 write 操作上不阻塞，网络库必须要给每个 TCP connection 配置 output buffer。
（2）TcpConnection 必须要有 input buffer　
   TCP 是一个无边界的字节流协议，接收方必须要处理“收到的数据尚不构成一条完整的消息”和“一次收到两条消息的数据”等情况。一个常见的场景是，发送方
      send() 了两条 1kB 的消息（共 2kB），接收方收到数据的情况可能是：
    • 一次性收到 2kB 数据；
    • 分两次收到，第一次 600B，第二次 1400B；
    • 分两次收到，第一次 1400B，第二次 600B；
    • 分两次收到，第一次 1kB，第二次 1kB；
    • 分三次收到，第一次 600B，第二次 800B，第三次 600B；
    • 其他任何可能。一般而言，长度为 n 字节的消息分块到达的可能性有种。
   网络库在处理“socket 可读”事件的时候，必须一次性把 socket 里的数据读完（从操作系统 buffer 搬到应用层 buffer），否则会反复触发 POLLIN 事件，
      造成 busy-loop。那么网络库必然要应对“数据不完整”的情况，收到的数据先放到 input buffer 里，等构成一条完整的消息再通知程序的业务逻辑。这通
      常是 codec 的职责，见 §7.3“Boost.Asio 的聊天服务器”中的“ TCP 分包”的论述与代码。所以，在 TCP 网络编程中，网络库必须要给每个 TCP connection
      配置 input buffer。
muduo EventLoop 采用的是 epoll(4)level trigger，而不是 edge trigger。一是为了与传统的 poll(2) 兼容，因为在文件描述符数目较少，活动文件
   描述符比例较高时，epoll(4)不见得比poll(2)更高效，必要时可以在进程启动时切换 Poller。二是 level trigger 编程更容易，以往
   select(2)/poll(2) 的经验都可以继续用，不可能发生漏掉事件的 bug。三是读写的时候不必等候出现 EAGAIN，可以节省系统调用次数，降低延迟。
所有 muduo 中的 IO 都是带缓冲的 IO（buffered IO），你不会自己去 read() 或 write() 某个 socket，只会操作 TcpConnection 的 input buffer
   和 output buffer。更确切地说，是在 onMessage() 回调里读取 input buffer；调用 TcpConnection::send() 来间接操作 output buffer，一般
   不会直接操作 output buffer。
*/

namespace muduo
{
namespace net
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
///
/**Buffer 仿 NettyChannelBuffer 的 bufferclass，数据的读写通过 buffer 进行。
 *   用户代码不需要调用 read(2)/write(2)，只需要处理收到的数据和准备好要发送的数据。*/
class Buffer : public muduo::copyable
{
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
    : buffer_(kCheapPrepend + initialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend)
  {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  // implicit copy-ctor, move-ctor, dtor and assignment are fine
  // NOTE: implicit move-ctor is added in g++ 4.6

  void swap(Buffer& rhs)
  {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  size_t readableBytes() const
  { return writerIndex_ - readerIndex_; }

  size_t writableBytes() const
  { return buffer_.size() - writerIndex_; }

  size_t prependableBytes() const
  { return readerIndex_; }

  const char* peek() const
  { return begin() + readerIndex_; }

  const char* findCRLF() const
  {
    // FIXME: replace with memmem()?
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
  }

  const char* findCRLF(const char* start) const
  {
    assert(peek() <= start);
    assert(start <= beginWrite());
    // FIXME: replace with memmem()?
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
  }

  const char* findEOL() const
  {
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  const char* findEOL(const char* start) const
  {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char*>(eol);
  }

  // retrieve returns void, to prevent
  // string str(retrieve(readableBytes()), readableBytes());
  // the evaluation of two functions are unspecified
  void retrieve(size_t len)
  {
    assert(len <= readableBytes());
    if (len < readableBytes())
    {
      readerIndex_ += len;
    }
    else
    {
      retrieveAll();
    }
  }

  void retrieveUntil(const char* end)
  {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }

  void retrieveInt64()
  {
    retrieve(sizeof(int64_t));
  }

  void retrieveInt32()
  {
    retrieve(sizeof(int32_t));
  }

  void retrieveInt16()
  {
    retrieve(sizeof(int16_t));
  }

  void retrieveInt8()
  {
    retrieve(sizeof(int8_t));
  }

  void retrieveAll()
  {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  string retrieveAllAsString()
  {
    return retrieveAsString(readableBytes());
  }

  string retrieveAsString(size_t len)
  {
    assert(len <= readableBytes());
    string result(peek(), len);
    retrieve(len);
    return result;
  }

  StringPiece toStringPiece() const
  {
    return StringPiece(peek(), static_cast<int>(readableBytes()));
  }

  void append(const StringPiece& str)
  {
    append(str.data(), str.size());
  }

  void append(const char* /*restrict*/ data, size_t len)
  {
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    hasWritten(len);
  }

  void append(const void* /*restrict*/ data, size_t len)
  {
    append(static_cast<const char*>(data), len);
  }

  void ensureWritableBytes(size_t len)
  {
    if (writableBytes() < len)
    {
      makeSpace(len);
    }
    assert(writableBytes() >= len);
  }

  char* beginWrite()
  { return begin() + writerIndex_; }

  const char* beginWrite() const
  { return begin() + writerIndex_; }

  void hasWritten(size_t len)
  {
    assert(len <= writableBytes());
    writerIndex_ += len;
  }

  void unwrite(size_t len)
  {
    assert(len <= readableBytes());
    writerIndex_ -= len;
  }

  ///
  /// Append int64_t using network endian
  ///
  void appendInt64(int64_t x)
  {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof be64);
  }

  ///
  /// Append int32_t using network endian
  ///
  void appendInt32(int32_t x)
  {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof be32);
  }

  void appendInt16(int16_t x)
  {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof be16);
  }

  void appendInt8(int8_t x)
  {
    append(&x, sizeof x);
  }

  ///
  /// Read int64_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int64_t readInt64()
  {
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
  }

  ///
  /// Read int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t readInt32()
  {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }

  int16_t readInt16()
  {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }

  int8_t readInt8()
  {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }

  ///
  /// Peek int64_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int64_t)
  int64_t peekInt64() const
  {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof be64);
    return sockets::networkToHost64(be64);
  }

  ///
  /// Peek int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t peekInt32() const
  {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof be32);
    return sockets::networkToHost32(be32);
  }

  int16_t peekInt16() const
  {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof be16);
    return sockets::networkToHost16(be16);
  }

  int8_t peekInt8() const
  {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }

  ///
  /// Prepend int64_t using network endian
  ///
  void prependInt64(int64_t x)
  {
    int64_t be64 = sockets::hostToNetwork64(x);
    prepend(&be64, sizeof be64);
  }

  ///
  /// Prepend int32_t using network endian
  ///
  void prependInt32(int32_t x)
  {
    int32_t be32 = sockets::hostToNetwork32(x);
    prepend(&be32, sizeof be32);
  }

  void prependInt16(int16_t x)
  {
    int16_t be16 = sockets::hostToNetwork16(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x)
  {
    prepend(&x, sizeof x);
  }

  void prepend(const void* /*restrict*/ data, size_t len)
  {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+readerIndex_);
  }

  void shrink(size_t reserve)
  {
    // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
    Buffer other;
    other.ensureWritableBytes(readableBytes()+reserve);
    other.append(toStringPiece());
    swap(other);
  }

  size_t internalCapacity() const
  {
    return buffer_.capacity();
  }

  /// Read data directly into buffer.
  ///
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t readFd(int fd, int* savedErrno);

 private:

  char* begin()
  { return &*buffer_.begin(); }

  const char* begin() const
  { return &*buffer_.begin(); }

  void makeSpace(size_t len)
  {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
      // FIXME: move readable data
      buffer_.resize(writerIndex_+len);
    }
    else
    {
      // move readable data to the front, make space inside buffer
      assert(kCheapPrepend < readerIndex_);
      size_t readable = readableBytes();
      std::copy(begin()+readerIndex_,
                begin()+writerIndex_,
                begin()+kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

 private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  static const char kCRLF[];
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_BUFFER_H
