namespace cpp videolib
namespace go videolib

struct VideoDataInput {
    1:string dataId;        // 非空 数据ID
    2:string localFile;     // 非空 视频文件在本地的路径
}

struct CutConfig {
    1:i32 interval;     // 间隔多少秒截取一帧 
    2:i32 maxFrames;    // 最多截取多少帧
}

struct FrameInfo {
    1:binary content;   // 截取图片的二进制数据
    2:i32 timeAt;       // 截图图片在视频中的时间位置
}

struct CutFrameResult {
    1:bool succ;                // 是否成功
    2:string dataId;            // 数据ID
    3:list<FrameInfo> imgs;    // 截帧后的每一帧数据内容
    4:double duration;          // 输入视频总时长
}

service VideoProc {
    // 处理函数
    CutFrameResult cutFrame(1:VideoDataInput mediaData, 2:CutConfig config);
}
