// decorator.h
#pragma once

#include "component.h"

// 调味品
class CondimentDecorator : public IBeverage {
public :
    explicit CondimentDecorator(IBeverage *beverage) : m_pBeverage(beverage) {}

    string Name() override {
        return m_pBeverage->Name();
    }

    double Cost() override {
        return m_pBeverage->Cost();
    }

protected :
    IBeverage *m_pBeverage;
};