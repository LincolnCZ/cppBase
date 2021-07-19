// concrete_product.h
#pragma once

#include "product.h"

/********** 汽车 **********/
// 奔驰
class BenzCar : public ICar {
public:
    string Name() override {
        return "Benz Car";
    }
};

// 宝马
class BmwCar : public ICar {
public:
    string Name() override {
        return "Bmw Car";
    }
};

// 奥迪
class AudiCar : public ICar {
public:
    std::string Name() override {
        return "Audi Car";
    }
};

/********** 自行车 **********/
// 奔驰
class BenzBike : public IBike {
public:
    string Name() override {
        return "Benz Bike";
    }
};

// 宝马
class BmwBike : public IBike {
public:
    string Name() override {
        return "Bmw Bike";
    }
};

// 奥迪
class AudiBike : public IBike {
public:
    string Name() override {
        return "Audi Bike";
    }
};