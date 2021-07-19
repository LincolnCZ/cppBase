// concrete_product.h
#pragma once

#include "product.h"

// 奔驰汽车
class BenzCar : public ICar {
public:
    std::string Name() override {
        return "Benz Car";
    }
};

// 宝马汽车
class BmwCar : public ICar {
public:
    std::string Name() override {
        return "Bmw Car";
    }
};

// 奥迪汽车
class AudiCar : public ICar {
public:
    std::string Name() override {
        return "Audi Car";
    }
};
