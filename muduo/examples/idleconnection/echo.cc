#include "examples/idleconnection/echo.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <assert.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;


EchoServer::EchoServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       int idleSeconds)
  : server_(loop, listenAddr, "EchoServer"),
    connectionBuckets_(idleSeconds)
{
  server_.setConnectionCallback(
      std::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&EchoServer::onMessage, this, _1, _2, _3));
  loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
  connectionBuckets_.resize(idleSeconds);
  dumpConnectionBuckets("EchoServer construct");
}

void EchoServer::start()
{
  server_.start();
}

// 在连接建立时，创建一个Entry对象，把它放到timing wheel的队尾。另外，我们还需要把Entry的弱引用保存到TcpConnection的context里，
// 因为在收到数据的时候还要用到Entry。（思考题：如果TcpConnection::setContext保存的是强引用EntryPtr，会出现什么情况？
// connectionBuckets_里面存储的是强引用，如果TcpConnection::setContext保存的是强引用，会造成引用计数至少为1，无法触发Entry的析构函数）
void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  if (conn->connected())
  {
    EntryPtr entry(new Entry(conn));
    connectionBuckets_.back().insert(entry);
    dumpConnectionBuckets("connection");
    WeakEntryPtr weakEntry(entry);
    conn->setContext(weakEntry);
  }
  else
  {
    assert(!conn->getContext().empty());
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
    LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
  }
}

// 在收到消息时，从TcpConnection的context中取出Entry的弱引用，把它提升为强引用EntryPtr，然后放到当前的timing wheel队尾。
// （思考题：为什么要把Entry作为TcpConnection的context保存，如果这里再创建一个新的Entry会有什么后果？
// 创建一个新的Entry，则产生的entry和之前的指向不同的地址，这样connectionBuckets_.back().insert(entry)的插入是不同的。）
void EchoServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size()
           << " bytes at " << time.toString();
  conn->send(msg);

  assert(!conn->getContext().empty());
  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
  EntryPtr entry(weakEntry.lock());
  if (entry)
  {
    connectionBuckets_.back().insert(entry);
    dumpConnectionBuckets(msg);
  }
}

void EchoServer::onTimer()
{
  // 往队尾添加一个空的Bucket，这样circular_buffer会自动弹出队首的Bucket，并析构之。
  // 在析构Bucket的时候，会依次析构其中的EntryPtr对象，这样Entry的引用计数就不用我们去操心，C++的值语意会帮我们搞定一切。
  connectionBuckets_.push_back(Bucket());
  dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets(const std::string &tag) const
{
  LOG_INFO << "tag ="<< tag << ", size = " << connectionBuckets_.size();
  int idx = 0;
  for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
      bucketI != connectionBuckets_.end();
      ++bucketI, ++idx)
  {
    const Bucket& bucket = *bucketI;
    printf("[%d] len = %zd : ", idx, bucket.size());
    for (const auto& it : bucket)
    {
      bool connectionDead = it->weakConn_.expired();
      printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
          connectionDead ? " DEAD" : "");
    }
    puts("");
  }
}

