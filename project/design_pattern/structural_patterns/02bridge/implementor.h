// implementor.h
#pragma once

// 电器
class IElectricalEquipment {
public:
    virtual ~IElectricalEquipment() = default;

    // 打开
    virtual void PowerOn() = 0;

    // 关闭
    virtual void PowerOff() = 0;
};