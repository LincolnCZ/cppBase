// director.h
#pragma once

#include "builder.h"

// 指挥者
class Direcror {
public:
    static void Create(IBuilder *builder) {
        builder->BuildCpu();
        builder->BuildMainboard();
        builder->BuildRam();
        builder->BuildVideoCard();
    }
};