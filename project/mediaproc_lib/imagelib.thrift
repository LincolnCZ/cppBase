namespace cpp imagelib
namespace go imagelib

struct ImageInput {
    1:string dataId;       // 非空 数据ID
    2:binary content;      // 非空 数据内容
}

struct SplitResult {
    1:bool succ;             // 是否成功
    2:string dataId;         // 数据ID
    3:list<binary> imgs;     // 截取图片的二进制数据，png格式
}

service ImageProc {
    // 处理函数
    SplitResult splitWebp(1:ImageInput mediaData);
}
