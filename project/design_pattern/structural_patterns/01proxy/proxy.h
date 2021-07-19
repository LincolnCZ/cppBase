// proxy.h
#pragma once

#include "subject.h"
#include "real_subject.h"
#include <iostream>

// 代理点
class Proxy : public Subject {
public:
    Proxy() : m_realSubject(nullptr) {};

    ~Proxy() override { delete m_realSubject; }

    // 低于 50 不充
    void Do(int money) override {
        if (money >= 50) {
            if (m_realSubject == nullptr)
                m_realSubject = new RealSubject();
            m_realSubject->Do(money);
        } else {
            std::cout << "Sorry, too little money" << std::endl;
        }
    }

private:
    RealSubject *m_realSubject{};
};