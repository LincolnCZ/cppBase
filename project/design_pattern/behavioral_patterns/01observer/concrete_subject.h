// concrete_subject.h
#pragma once

#include "subject.h"
#include "observer.h"
#include <iostream>
#include <list>

using namespace std;

// 具体主题
class ConcreteSubject : public ISubject {
public:
    ConcreteSubject() { m_fPrice = 10.0; }

    void SetPrice(float price) {
        m_fPrice = price;
    }

    void Attach(IObserver *observer) override {
        m_observers.push_back(observer);
    }

    void Detach(IObserver *observer) override {
        m_observers.remove(observer);
    }

    void Notify() override {
        auto it = m_observers.begin();
        while (it != m_observers.end()) {
            (*it)->Update(m_fPrice);
            ++it;
        }
    }

private:
    list<IObserver *> m_observers;  // 观察者列表
    float m_fPrice;  // 价格
};