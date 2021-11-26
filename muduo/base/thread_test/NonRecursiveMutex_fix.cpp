#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include <vector>
#include <stdio.h>

using namespace muduo;

class Foo {
public:
    void doit() const;
};


typedef std::vector<Foo> FooList;
typedef std::shared_ptr<FooList> FooListPtr;
MutexLock mutex;
FooListPtr g_foos = std::make_shared<FooList>();


void post(const Foo &f) {
    printf("post\n");
    MutexLockGuard lock(mutex);
    printf("g_foos count :%ld\n", g_foos.use_count());
    if (!g_foos.unique()) {
        g_foos.reset(new FooList(*g_foos));
        printf("copy the whole list\n"); //
    }
    printf("after g_foos count :%ld\n", g_foos.use_count());
    assert(g_foos.unique());
    g_foos->push_back(f);
}

void traverse() {
    FooListPtr foos;
    {
        MutexLockGuard lock(mutex);
        foos = g_foos;
        assert(!g_foos.unique());
    }

    for (std::vector<Foo>::const_iterator it = foos->begin(); it != foos->end(); ++it) {
        printf("traverse");
        it->doit();
    }
}

void Foo::doit() const {
    Foo f;
    post(f);
}

int main() {
    Foo f;
    post(f);
    traverse();
}