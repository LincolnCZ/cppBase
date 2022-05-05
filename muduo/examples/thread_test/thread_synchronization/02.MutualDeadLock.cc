/// 这个例子存在两种死锁的可能：
///1)第一种
///  main()线程是先调用Inventory::printAll(#6)再调用Request::print(#5)，
///  而threadFunc()线程是先调用Request::~Request(#6)再调用Inventory::remove(#5)。
///  这两个调用序列对两个mutex的加锁顺序正好相反，于是造成了经典的死锁。
///2)第二种
///  main()线程是先调用Inventory::printAll(#6)再调用Request::print(#5)，
///  而threadFunc()线程先调用Request::process()，再调用Inventory::add()。
///  这两个调用序列对两个mutex的加锁顺序正好相反，于是造成了经典的死锁。
///
/// 析构函数中存在race condition
///

#include <set>
#include <cstdio>
#include <unistd.h>
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

class Request;

class Inventory
{
 public:
  void add(Request* req)
  {
    muduo::MutexLockGuard lock(mutex_);
    requests_.insert(req);
  }

  void remove(Request* req) __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
    requests_.erase(req);
  }

  void printAll() const;

 private:
  mutable muduo::MutexLock mutex_;
  std::set<Request*> requests_;
};

Inventory g_inventory;

class Request
{
 public:
  void process() // __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
    g_inventory.add(this);
    // ...
  }

  ~Request() __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
    sleep(1);
    g_inventory.remove(this);
  }

  void print() const __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
    // ...
  }

 private:
  mutable muduo::MutexLock mutex_;
};

void Inventory::printAll() const
{
  muduo::MutexLockGuard lock(mutex_);
  sleep(1);
  for (std::set<Request*>::const_iterator it = requests_.begin();
      it != requests_.end();
      ++it)
  {
    (*it)->print();
  }
  printf("Inventory::printAll() unlocked\n");
}

/* 解决死锁的一种方法：把requests_复制一份，在临界区之外遍历这个副本。
 *
void Inventory::printAll() const
{
  std::set<Request*> requests
  {
    muduo::MutexLockGuard lock(mutex_);
    requests = requests_;
  }
  for (std::set<Request*>::const_iterator it = requests.begin();
      it != requests.end();
      ++it)
  {
    (*it)->print();
  }
}
*/

void threadFunc()
{
  Request* req = new Request;
  req->process();
  delete req;
}

int main()
{
  muduo::Thread thread(threadFunc);
  thread.start();
  usleep(500 * 1000);
  g_inventory.printAll();
  thread.join();
}
