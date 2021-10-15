#pragma once

#include <string>
#include <vector>

struct CutFrameInfo {
    std::string content;
    int32_t timeAt;
};

/** 参数说明：
 @param dataId:标识当前的请求
 @param input:输入视频文件的路径
 @param output:输出结果
 @param frameInterval:每隔frameInterval帧截取一帧
 @param timeInterval:每隔timeInterval秒截取一帧
 @param maxFrames:最大截取的帧数
 @param duration:返回视频的时长
 */
extern bool
cutFrames(const std::string &dataId, const std::string &input, std::vector<CutFrameInfo> &output, int frameInterval,
          int timeInterval, int maxFrames, double &duration);