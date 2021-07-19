// component.h
#pragma once

#include <string>

using namespace std;

// 所有饮料的基类
class IBeverage {
public:
    virtual string Name() = 0;  // 名称
    virtual double Cost() = 0;  // 价钱
};
