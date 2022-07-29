///


#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include <vector>
#include <cstdio>
#include <memory>

using namespace muduo;

class Foo {
public:
    void doit() const;
};

typedef std::vector<Foo> FooList;
typedef std::shared_ptr<FooList> FooListPtr;
FooListPtr g_foos;
MutexLock mutex;

/// 关键看write端的post()该如何写。按照前面的描述，如果g_foos.unique()为true，我们可以放心地在原地（in-place）修改FooList。
/// 如果g_foos.unique()为false，说明这时别的线程正在读取FooList，我们不能原地修改，而是复制一份，在副本上修改。这样就避免了死锁。
///  • 注意这里临界区包括整个函数，其他写法都是错的。读者可以试着运行这个程序，看看什么时候会打印(copy the whole list)。
//void post(const Foo &f) {
//    printf("post\n");
//    MutexLockGuard lock(mutex);
//    if (!g_foos.unique()) {
//        g_foos.reset(new FooList(*g_foos)); // 复制了一份原始的数据
//        printf("copy the whole list\n");
//    }
//    assert(g_foos.unique());
//    g_foos->push_back(f);
//}

/// 错误一：直接修改 g_foos 所指的 FooListvoid
/// 结果：因为是直接 g_foos 中添加元素，导致 traverse 函数中 foos->end()一直更新，结果就是陷入死循环调用post()函数。
//void post(const Foo& f){
//    printf("post\n");
//    MutexLockGuard lock(mutex);
//    g_foos->push_back(f);
//}

/// 错误二：试图缩小临界区，把 copying 移出临界区
/// 结果：
void post(const Foo &f) {
    printf("post\n");
    FooListPtr newFoos(new FooList(*g_foos));
    newFoos->push_back(f);
    MutexLockGuard lock(mutex);
    g_foos = newFoos; // 或者 g_foos.swap(newFoos);
}

//// 错误三：把临界区拆成两个小的，把 copying 放到临界区之外
//void post(const Foo& f){
//    FooListPtr oldFoos;
//    {
//        MutexLockGuard lock(mutex);
//        oldFoos=g_foos;
//    }
//    FooListPtr newFoos(new FooList(*oldFoos));
//    newFoos->push_back(f);
//    MutexLockGuard lock(mutex);
//    g_foos=newFoos; // 或者 g_foos.swap(newFoos);
//}

/// 在read端，用一个栈上局部FooListPtr变量当做“观察者”，它使得g_foos的引用计数增加。
/// traverse()函数的临界区内只读了一次共享变量g_foos（这里多线程并发读写shared_ptr，因此必须用mutex保护），比原来的写法大为缩短。
/// 而且多个线程同时调用traverse()也不会相互阻塞。
void traverse() {
    FooListPtr foos;
    {
        MutexLockGuard lock(mutex);
        foos = g_foos;
        assert(!g_foos.unique());
    }

    // assert(!foos.unique()); this may not hold

    for (std::vector<Foo>::const_iterator it = foos->begin();
         it != foos->end(); ++it) {
        it->doit();
    }
}

void Foo::doit() const {
    Foo f;
    post(f); // 如果这里又调用了 post() 函数
}

int main() {
    g_foos.reset(new FooList);
    Foo f;
    post(f);
    traverse();
}

