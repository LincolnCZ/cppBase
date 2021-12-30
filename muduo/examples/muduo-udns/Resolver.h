#ifndef MUDUO_UDNS_RESOLVER_H
#define MUDUO_UDNS_RESOLVER_H

#include <muduo/base/Timestamp.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/noncopyable.h>
#include <functional>
#include <memory>

extern "C"
{
struct dns_ctx;
struct dns_rr_a4;
}

namespace muduo
{
namespace net
{
class Channel;
class EventLoop;

class Resolver : muduo::noncopyable
{
 public:
  typedef std::function<void(const InetAddress&)> Callback;

  Resolver(EventLoop* loop);
  Resolver(EventLoop* loop, const InetAddress& nameServer);
  ~Resolver();

  void start();

  bool resolve(const StringPiece& hostname, const Callback& cb);

 private:

  struct QueryData
  {
    Resolver* owner;
    Callback callback;
    QueryData(Resolver* o, const Callback& cb)
      : owner(o), callback(cb)
    {
    }
  };

  void onRead(Timestamp t);
  void onTimer();
  void onQueryResult(struct dns_rr_a4 *result, const Callback& cb);
  static void dns_query_a4(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data);

  EventLoop* loop_;
  dns_ctx* ctx_;
  int fd_;
  bool timerActive_;
  std::unique_ptr<Channel> channel_;
};

}
}
#endif  // MUDUO_UDNS_RESOLVER_H
