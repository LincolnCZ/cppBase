#include <algorithm>
#include <vector>
#include <cstdio>
#include <memory>
#include "muduo/base/Mutex.h"

/// 修改：将Observable 中的std::vector<Observer*> observers_; 改成std::vector<std::weak_ptr<Observer> > observers_;
///
/// 思考：如果把L32改为vector<shared_ptr<Observer>> observers_;，会有什么后果？
///      observers_的类型改为vector<shared_ptr<Observer> >，那么除非手动调用unregister()，否则Observer对象永远不会析构。
///      即便它的析构函数会调用unregister()，但是不去unregister()就不会调用Observer的析构函数，这变成了鸡与蛋的问题。
///
/// 存在问题：把Observer*替换为weak_ptr<Observer>部分解决了Observer模式的线程安全，但还有以下几个疑点。
/// • 侵入性　强制要求Observer必须以shared_ptr来管理。
/// • 不是完全线程安全  Observer的析构函数会调用subject_->unregister(this)，万一subject_已经不复存在了呢？
///    为了解决它，又要求Observable本身是用shared_ptr管理的，并且subject_多半是个weak_ptr<Observable>。
/// • 锁争用（lock contention）　即 Observable 的三个成员函数都用了互斥器来同步，这会造成 register_() 和 unregister() 等待 notifyObservers()，
///    而后者的执行时间是无上限的，因为它同步回调了用户提供的 update() 函数。我们希望 register_() 和 unregister() 的执行时间不会超过某个固定的上限，
///    以免殃及无辜群众。
/// • 死锁　万一 update() 虚函数中调用了 (un)register 呢？如果 mutex_ 是不可重入的，那么会死锁；如果 mutex_ 是可重入的，
///    程序会面临迭代器失效（core dump是最好的结果），因为 vector observers_ 在遍历期间被意外地修改了。这个问题乍看起来似乎没有解决办法，
///    除非在文档里做要求。（一种办法是：用可重入的 mutex_，把容器换为 std::list，并把 ++it 往前挪一行。）
///    我个人倾向于使用不可重入的 mutex，例如 Pthreads 默认提供的那个，因为“要求 mutex 可重入”本身往往意味着设计上出了问题（§2.1.1）。
///    Java 的 intrinsic lock 是可重入的，因为要允许 synchronized 方法相互调用（派生类调用基类的同名 synchro-nized 方法），我觉得这也是无奈之举。
///
/// 解决方法：参见04.SignalSlot.h 文件

class Observable;

class Observer : public std::enable_shared_from_this<Observer> {
public:
    virtual ~Observer();

    virtual void update() = 0;

    void observe(Observable *s);

protected:
    Observable *subject_;
};

class Observable // not 100% thread safe!
{
public:
    void register_(std::weak_ptr<Observer> x); // 参数类型可用 const weak_ptr<Observer>&
    // void unregister(std::weak_ptr<Observer> x); // 不需要它

    void notifyObservers() {
        muduo::MutexLockGuard lock(mutex_);
        Iterator it = observers_.begin();
        while (it != observers_.end()) {
            std::shared_ptr<Observer> obj(it->lock());  // 尝试提升，这一步是线程安全的
            if (obj) {
                // 提升成功，现在引用计数值至少为 2（想想为什么？）
                obj->update(); // 没有竞态条件，因为 obj 在栈上，对象不可能在本作用域内销毁
                ++it;
            } else {
                printf("notifyObservers() erase\n");
                // 对象已经销毁，从容器中拿掉 weak_ptr
                it = observers_.erase(it);
            }
        }
    }

private:
    mutable muduo::MutexLock mutex_;
    std::vector<std::weak_ptr<Observer> > observers_;
    typedef std::vector<std::weak_ptr<Observer> >::iterator Iterator;
};

Observer::~Observer() {
    // subject_->unregister(this);
}

void Observer::observe(Observable *s) {
    s->register_(shared_from_this());
    subject_ = s;
}

void Observable::register_(std::weak_ptr<Observer> x) {
    observers_.push_back(x);
}

//void Observable::unregister(std::weak_ptr<Observer> x)
//{
//  Iterator it = std::find(observers_.begin(), observers_.end(), x);
//  observers_.erase(it);
//}

// ---------------------

class Foo : public Observer {
    virtual void update() {
        printf("Foo::update() %p\n", this);
    }
};

int main() {
    Observable subject;
    {
        std::shared_ptr<Foo> p(new Foo);
        p->observe(&subject);
        subject.notifyObservers();
    }
    subject.notifyObservers();
}

