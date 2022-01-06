#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include <vector>
#include <cstdio>
#include <memory>

using namespace muduo;

class Foo
{
 public:
  void doit() const;
};

typedef std::vector<Foo> FooList;
typedef std::shared_ptr<FooList> FooListPtr;
FooListPtr g_foos;
MutexLock mutex;

void post(const Foo& f)
{
  printf("post\n");
  MutexLockGuard lock(mutex);
  if (!g_foos.unique())
  {
    g_foos.reset(new FooList(*g_foos)); // 复制了一份原始的数据
    printf("copy the whole list\n");
  }
  assert(g_foos.unique());
  g_foos->push_back(f);
}

void traverse()
{
  FooListPtr foos;
  {
    MutexLockGuard lock(mutex);
    foos = g_foos;
    assert(!g_foos.unique());
  }

  // assert(!foos.unique()); this may not hold

  for (std::vector<Foo>::const_iterator it = foos->begin();
      it != foos->end(); ++it)
  {
    it->doit();
  }
}

void Foo::doit() const
{
  Foo f;
  post(f);
}

int main()
{
  g_foos.reset(new FooList);
  Foo f;
  post(f);
  traverse();
}

