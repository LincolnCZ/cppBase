// concrete_factory.h
#pragma once

#include "factory.h"
#include "concrete_product.h"

// 奔驰工厂
class BenzFactory : public AFactory {
public:
    ICar *CreateCar() override {
        return new BenzCar();
    }
};

// 宝马工厂
class BmwFactory : public AFactory {
public:
    ICar *CreateCar() override {
        return new BmwCar();
    }
};

// 奥迪工厂
class AudiFactory : public AFactory {
public:
    ICar *CreateCar() override {
        return new AudiCar();
    }
};