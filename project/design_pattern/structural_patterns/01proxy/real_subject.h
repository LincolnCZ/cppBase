// real_subject.h
#pragma once

#include "subject.h"
#include <iostream>

class RealSubject : public Subject {
public:
    void Do(int money) override {
        std::cout << "Recharge " << money;
    }
};