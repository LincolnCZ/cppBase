// abstraction.h
#pragma once

#include "implementor.h"

// 开关
class ISwitch {
public:
    explicit ISwitch(IElectricalEquipment *ee) { m_pEe = ee; }

    virtual ~ISwitch() = default;

    // 打开电器
    virtual void On() = 0;

    // 关闭电器
    virtual void Off() = 0;

protected:
    IElectricalEquipment *m_pEe;
};