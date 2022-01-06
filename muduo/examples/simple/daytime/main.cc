#include "examples/simple/daytime/daytime.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int main()
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(2013);
  DaytimeServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}

//用netcat扮演客户端，运行结果如下：
//$ nc 127.0.0.1 2013
//2011-02-02 03:31:26.622647    # 服务器返回的时间字符串，UTC时区