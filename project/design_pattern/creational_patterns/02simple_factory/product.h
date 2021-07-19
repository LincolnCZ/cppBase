// product.h
#pragma once

#include <string>

// 汽车接口
class ICar {
public:
    virtual ~ICar() = default;

    virtual std::string Name() = 0;  // 汽车名称
};