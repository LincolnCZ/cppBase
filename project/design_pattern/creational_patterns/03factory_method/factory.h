// factory.h
#pragma once

#include "product.h"

// 工厂接口
class AFactory {
public:
    virtual ~AFactory() = default;

    virtual ICar *CreateCar() = 0;  // 生产汽车
};
