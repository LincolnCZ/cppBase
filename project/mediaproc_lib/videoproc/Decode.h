#pragma once

#include <string>
#include <vector>

struct CutFrameInfo {
    std::string content;
    int32_t timeAt;
};

extern bool
cutFrames(const std::string &dataId, const std::string &input, std::vector<CutFrameInfo> &output, int frameInterval,
          int timeInterval, int maxFrames, double &duration);