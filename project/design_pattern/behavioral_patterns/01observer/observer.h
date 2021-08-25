// observer.h
#pragma once

// 抽象观察者
class IObserver {
public:
    virtual void Update(float price) = 0;  // 更新价格
};