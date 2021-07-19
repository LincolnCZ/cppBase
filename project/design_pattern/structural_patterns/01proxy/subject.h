// subject.h
#pragma once

class Subject {
public:
    virtual ~Subject() = default;

    virtual void Do(int money) = 0;  // 充值
};