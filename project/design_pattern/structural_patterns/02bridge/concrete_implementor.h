// concrete_implementor.h
#pragma once

#include "implementor.h"
#include <iostream>

// 电灯
class Light : public IElectricalEquipment {
public:
    // 开灯
    void PowerOn() override {
        std::cout << "Light is on." << std::endl;
    }

    // 关灯
    void PowerOff() override {
        std::cout << "Light is off." << std::endl;
    }
};

// 风扇
class Fan : public IElectricalEquipment {
public:
    // 打开风扇
    void PowerOn() override {
        std::cout << "Fan is on." << std::endl;
    }

    // 关闭风扇
    void PowerOff() override {
        std::cout << "Fan is off." << std::endl;
    }
};