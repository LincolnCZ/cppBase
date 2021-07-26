# mediaproc_lib

音频和视频处理库。可以通过本地socket file，进行thrift调用，方便跨语言调用。

接入自动构建：`https://ci.yy.com/jenkins2/job/server-svr_comm_tech-aladdin-open-api-micro_services-mediaproc_lib/`

构建目标：

- 音频处理：服务 audioproc_server_d 静态库 libaudioproc.a
- 视频处理：服务 videoproc_server_d
- 图片处理：服务 imageproc_server_d

## 编译环境

推荐使用docker镜像进行编译。也可按照下面依赖库列表，手动安装依赖项。

完整编译命令，包括测试程序：

``` bash
mkdir build
cd build
cmake -DHAS_EXAMPLE=1 ..
make
```

### 依赖第三方库

#### ffmpeg

ffmpeg 使用版本 3.4.7，需要的依赖项：

`apt install libspeex-dev libfdk-aac-dev`

编译配置命令：

`./configure --enable-shared --disable-doc --disable-debug --enable-libspeex --enable-libfdk-aac`

#### libwebp

libwebp 使用版本 1.1.0，需要的依赖项：

`apt install libpng-dev`

#### thrift

thrift 使用版本 0.11.0，无第三方依赖。