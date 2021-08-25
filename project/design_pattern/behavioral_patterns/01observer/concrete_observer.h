// concrete_observer.h
#pragma once

#include "observer.h"
#include <iostream>
#include <string>

using namespace std;

// 具体观察者
class ConcreteObserver : public IObserver {
public:
    explicit ConcreteObserver(const string &name) { m_strName = name; }

    void Update(float price) override {
        cout << m_strName << " - price: " << price << "\n";
    }

private:
    string m_strName;  // 名字
};