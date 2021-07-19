// product.h
#pragma once

#include <string>

using namespace std;

// 汽车接口
class ICar {
public:
    virtual ~ICar() = default;

    virtual string Name() = 0;  // 汽车名称
};

// 自行车接口
class IBike {
public:
    virtual ~IBike() = default;

    virtual string Name() = 0;  // 自行车名称
};