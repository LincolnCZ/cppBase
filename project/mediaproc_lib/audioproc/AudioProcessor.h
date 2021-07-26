#pragma once

#include <string>
#include <vector>

class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();
    AudioProcessor(const AudioProcessor&) = delete;
    AudioProcessor &operator=(const AudioProcessor&) = delete;

    bool init(const char *taskId, const std::string &input, bool inputPath);
    const char *formatName() const;
    double duration() const;

    // 计算并获取准确的时间长度
    double calcDuration();

    bool splitAudio(std::vector<std::string> &outbuf, int segtime);
    // 解码输入音频为pcm，并进行重采样
    bool decodeAudio(int sampleRate, int channels, std::string &outbuf);
    // 转换输入音频格式为 m4a 格式，可以指定采样率和声道
    bool convertAudio(int sampleRate, int channels, const std::string &format, std::string &outbuf);

private:
    struct AudioProcessorImpl;
    AudioProcessorImpl *_impl;
};
