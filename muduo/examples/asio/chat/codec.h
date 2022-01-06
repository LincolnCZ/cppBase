#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include "muduo/base/Logging.h"
#include "muduo/net/Buffer.h"
#include "muduo/net/Endian.h"
#include "muduo/net/TcpConnection.h"

class LengthHeaderCodec : muduo::noncopyable
{
 public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const muduo::string& message,
                                muduo::Timestamp)> StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
    : messageCallback_(cb)
  {
  }

  // 本聊天服务的消息格式非常简单，“消息”本身是一个字符串，每条消息有一个4字节的头部，以网络序存放字符串的长度。
  // 消息之间没有间隙，字符串也不要求以'\0'结尾。
  //
  // 以两条消息的字节流为例：
  //    0x00, 0x00, 0x00, 0x05, 'h', 'e', 'l', 'l', 'o',
  //    0x00, 0x00, 0x00, 0x08, 'c', 'h', 'e', 'n', 's', 'h', 'u', 'o'
  // 假设数据最终都全部到达，onMessage()至少要能正确处理以下各种数据到达的次序，每种情况下messageCallback_都应该被调用两次：
  // 1.每次收到一个字节的数据，onMessage()被调用21次；
  // 2.数据分两次到达，第一次收到2个字节，不足消息的长度字段；
  // 3.数据分两次到达，第一次收到4个字节，刚好够长度字段，但是没有body；
  // 4.数据分两次到达，第一次收到8个字节，长度完整，但body不完整；
  // 5.数据分两次到达，第一次收到9个字节，长度完整，body也完整；
  // 6.数据分两次到达，第一次收到10个字节，第一条消息的长度完整、body也完整，第二条消息长度不完整；
  // 7.请自行移动和增加分割点，验证各种情况；一共有超过100万种可能（ 2^(21-1) ）。
  // 8.数据一次就全部到达，这时必须用while循环来读出两条消息，否则消息会堆积在Buffer中。
  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime)
  {
    // while循环来反复读取数据，直到Buffer中的数据不够一条完整的消息。
    // 请读者思考，如果换成if (buf->readableBytes()>=kHeaderLen)会有什么后果。
    while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
    {
      // FIXME: use Buffer::peekInt32()
      const void* data = buf->peek();
      // 在某些不支持非对齐内存访问的体系结构上会造成SIGBUScore dump，读取消息长度应该改用Buffer::peekInt32()。
      int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
      const int32_t len = muduo::net::sockets::networkToHost32(be32);
      if (len > 65536 || len < 0)
      {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();  // FIXME: disable reading
        break;
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        muduo::string message(buf->peek(), len);
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(len);
      }
      else
      {
        break;
      }
    }
  }

  // FIXME: TcpConnectionPtr
  void send(muduo::net::TcpConnection* conn,
            const muduo::StringPiece& message)
  {
    muduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
