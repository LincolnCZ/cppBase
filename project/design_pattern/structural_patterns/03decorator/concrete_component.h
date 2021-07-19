// concrete_component.h
#pragma once

#include "component.h"

/********** 具体的饮料（咖啡）**********/

// 黑咖啡，属于混合咖啡
class HouseBlend : public IBeverage {
public:
    string Name() override {
        return "HouseBlend";
    }

    double Cost() override {
        return 30.0;
    }
};

// 深度烘培咖啡豆
class DarkRoast : public IBeverage {
public:
    string Name() override {
        return "DarkRoast";
    }

    double Cost() override {
        return 28.5;
    }
};