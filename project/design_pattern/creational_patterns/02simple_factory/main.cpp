// main.cpp
#include "factory.h"
#include "product.h"
#include <iostream>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p){delete(p); (p)=NULL;} }
#endif

int main() {
    // 工厂
    auto *pFactory = new Factory();

    // 奔驰汽车
    ICar *pCar = pFactory->CreateCar(Factory::BENZ_CAR);
    std::cout << pCar->Name() << std::endl;
    SAFE_DELETE(pCar);

    // 宝马汽车
    pCar = pFactory->CreateCar(Factory::BMW_CAR);
    std::cout << pCar->Name() << std::endl;
    SAFE_DELETE(pCar);

    // 奥迪汽车
    pCar = pFactory->CreateCar(Factory::AUDI_CAR);
    std::cout << pCar->Name() << std::endl;
    SAFE_DELETE(pCar);

    return 0;
}