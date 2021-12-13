#include <algorithm>
#include <vector>
#include <cstdio>

// 存在3个问题：
// (1) 见代码中位置
// (2)   • 线程A执行到 ~Observer()，还没有来得及unregister本对象。
//       • 线程B执行到 notifyObservers()函数，用 x->update()，x正好指向是正在析构的对象。
//      这时悲剧又发生了，既然x所指的Observer对象正在析构，调用它的任何非静态成员函数都是不安全的，何况是虚函数。
//      更糟糕的是，Observer是个基类，执行到~Observer()时，派生类对象已经析构掉了，这时候整个对象处于将死未死的状态，core dump恐怕是最幸运的结果。
// (3) 见代码中位置

class Observable;

class Observer
{
 public:
  virtual ~Observer();
  virtual void update() = 0;

  void observe(Observable* s);

 protected:
  Observable* subject_;
};

class Observable
{
 public:
  void register_(Observer* x);
  void unregister(Observer* x);

  void notifyObservers()
  {
    for (size_t i = 0; i < observers_.size(); ++i)
    {
      Observer* x = observers_[i];
      if (x) {
        x->update(); // (3)当Observable通知每一个Observer时，它从何得知Observer对象x还活着？
      }
    }
  }

 private:
  std::vector<Observer*> observers_;
};

Observer::~Observer()
{
  subject_->unregister(this); // (1)如何得知subject_还活着？
}

void Observer::observe(Observable* s)
{
  s->register_(this);
  subject_ = s;
}

void Observable::register_(Observer* x)
{
  observers_.push_back(x);
}

void Observable::unregister(Observer* x)
{
  std::vector<Observer*>::iterator it = std::find(observers_.begin(), observers_.end(), x);
  if (it != observers_.end())
  {
    std::swap(*it, observers_.back());
    observers_.pop_back();
  }
}

// ---------------------

class Foo : public Observer
{
  virtual void update()
  {
    printf("Foo::update() %p\n", this);
  }
};

int main()
{
  Foo* p = new Foo;
  Observable subject;
  p->observe(&subject);
  subject.notifyObservers();
  delete p;
  subject.notifyObservers();
}
