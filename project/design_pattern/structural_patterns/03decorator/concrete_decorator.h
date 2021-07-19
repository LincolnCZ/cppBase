// concrete_decorator.h
#pragma once

#include "decorator.h"

/********** 具体的饮料（调味品）**********/

// 奶油
class Cream : public CondimentDecorator {
public:
    explicit Cream(IBeverage *beverage) : CondimentDecorator(beverage) {}

    string Name() override {
        return m_pBeverage->Name() + " Cream";
    }

    double Cost() override {
        return m_pBeverage->Cost() + 3.5;
    }
};

// 摩卡
class Mocha : public CondimentDecorator {
public:
    explicit Mocha(IBeverage *beverage) : CondimentDecorator(beverage) {}

    string Name() override {
        return m_pBeverage->Name() + " Mocha";
    }

    double Cost() override {
        return m_pBeverage->Cost() + 2.0;
    }
};

// 糖浆
class Syrup : public CondimentDecorator {
public:
    explicit Syrup(IBeverage *beverage) : CondimentDecorator(beverage) {}

    string Name() override {
        return m_pBeverage->Name() + " Syrup";
    }

    double Cost() override {
        return m_pBeverage->Cost() + 3.0;
    }
};