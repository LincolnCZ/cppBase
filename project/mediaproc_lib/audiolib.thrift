namespace cpp audiolib
namespace go audiolib

struct AudioDataInput {
    1:string dataId;       // 非空 数据ID
    2:binary content;      // 非空 数据内容
    3:string dataType;     // 非空 数据类型。 BINARY 二进制数据，PATH 本地文件
    4:string extra;        // 允许空 数据格式相关参数, 如pcm数据的采样率, 声道数等. 采用json格式
}

struct SplitResult {
    1:bool succ;                // 是否成功
    2:string dataId;            // 数据ID
    3:list<binary> contents;    // 切割后的音频数据内容
    4:double duration;          // 输入音频总时长
    5:i32 segtime;              // 切片长度
}

struct AudioOutput {
    1:bool succ;                // 是否成功
    2:string dataId;            // 数据ID
    3:binary content;           // 解码后的音频数据内容
    4:double duration;          // 输入音频总时长
    5:string format;            // 输出音频格式
}

service AudioProc {
    // 处理函数
    SplitResult splitAudio(1:AudioDataInput mediaData, 2:i32 lower, 3:i32 upper);
    AudioOutput decodeAudio(1:AudioDataInput mediaData, 2:i32 sampleRate, 3:i32 channels);
    AudioOutput convertAudio(1:AudioDataInput mediaData, 2:i32 sampleRate, 3:i32 channels, 4:string format);
}
