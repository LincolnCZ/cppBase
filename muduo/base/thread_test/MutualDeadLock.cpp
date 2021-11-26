#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include <set>
#include <unistd.h>

class Request;

class Inventory {
public:
    void add(Request *req) {
        muduo::MutexLockGuard lock(mutex_);
        requests_.insert(req);
    }

    void remove(Request *req) __attribute__ ((noinline)) {
        muduo::MutexLockGuard lock(mutex_);
        requests_.erase(req);
    }

    void printAll() const;

private:
    mutable muduo::MutexLock mutex_;
    std::set<Request *> requests_;
};

Inventory g_inventory;

class Request {
public:
    void process() // __attribute__ ((noinline))
    {
        muduo::MutexLockGuard lock(mutex_);
        g_inventory.add(this);
        // ...
    }

    ~Request() __attribute__ ((noinline)) {
        muduo::MutexLockGuard lock(mutex_);
        sleep(1);
        g_inventory.remove(this);
    }

    void print() const __attribute__ ((noinline)) {
        muduo::MutexLockGuard lock(mutex_);
        // ...
    }

private:
    mutable muduo::MutexLock mutex_;
};

void Inventory::printAll() const {
    muduo::MutexLockGuard lock(mutex_);
    sleep(1);
    for (std::set<Request *>::const_iterator it = requests_.begin();
         it != requests_.end();
         ++it) {
        (*it)->print();
    }
    printf("Inventory::printAll() unlocked\n");
}

/**

// 解决死锁的一种方法，把requests_复制一份，在临界区之外遍历这个副本。
// 这么做有一个明显的缺点，它复制了整个std::set中的每个元素，开销可能会比较大。
void Inventory::printAll() const
{
  std::set<Request*> requests
  {
    muduo::MutexLockGuard lock(mutex_);
    requests = requests_;
  }

  //  遍历局部变量 requests，调用 Request::print()
  for (std::set<Request*>::const_iterator it = requests.begin();
      it != requests.end();
      ++it)
  {
    (*it)->print();
  }
}
*/

void threadFunc() {
    Request *req = new Request;
    req->process();
    delete req;
}

int main() {
    muduo::Thread thread(threadFunc);
    thread.start();
    usleep(500 * 1000);
    g_inventory.printAll();
    thread.join();
}