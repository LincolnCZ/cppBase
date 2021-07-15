# aladdin_cover_check_cpp

阿拉丁异步服务，视频质量检测服务c++版本


ailib目录下存放的是ai提供的so、头文件和使用的demo。算法代码git目录是：https://git.yy.com/ai/cv/video-understanding/cover_quality_check_api


算法主要依赖：libtorch、OpenCV和trt7

## 编译说明

### 编译机器
目前是在一台阿里云的v100机器上进行编译的，机器ip为 112.74.15.192，该机器目前只给AI同学做日常的训练使用。

### 编译操作
docker 镜像位置：/data/linchengzhong/docker_imamge/classifition_quality_cover_image_1.0.tar。

libtorch、OpenCV等ai相关的依赖库位置：/data/linchengzhong/picture_to_cartoon_libs

新机器中可以使用以下命令加载镜像：

```bash
# 新机器中加载对应的镜像
docker load < classifition_quality_cover_image_1.0.tar
# 之后使用docker run启动容器
...
```

在112.74.15.192机器中，可以直接进入已有的容器：

```bash
# 可以直接进入对应容器进行编译
docker exec -it ugatit_linchengzhong bash
cd /data

# 进入代码目录
cd aladdin_cover_check_cpp
mkdir build
cd build
cmake ..
make
```

成功执行完以上操作后，会在build目录下生产 aladdin_cover_check_cpp_d 可执行文件。

## 部署说明
das上包名：aladdin_cover_check_cpp

c++依赖库包名：aladdin_depends_cover_check

CUDA驱动、CUDA10.0环境安装、CUDNN安装，可参考脚本：install_cuda_cudnn.sh

```bash
#!/bin/bash

RED=\\e[1m\\e[31m
DARKRED=\\e[31m
GREEN=\\e[1m\\e[32m
DARKGREEN=\\e[32m
BLUE=\\e[1m\\e[34m
DARKBLUE=\\e[34m
YELLOW=\\e[1m\\e[33m
DARKYELLOW=\\e[33m
MAGENTA=\\e[1m\\e[35m
DARKMAGENTA=\\e[35m
CYAN=\\e[1m\\e[36m
DARKCYAN=\\e[36m
RESET=\\e[m

echo  -e "${GREEN} install cuda 10.0 ${RESET}"
mkdir -p /data/services/tmp_cuda/
wget http://113.107.239.242/linchengzhong/myapp/cuda_10.0.130_410.48_linux.run
chmod +x cuda_10.0.130_410.48_linux.run
./cuda_10.0.130_410.48_linux.run  --silent   --toolkit   --override -tmpdir /data/services/tmp_cuda/
rm -r /data/services/tmp_cuda/
rm -r cuda_10.0.130_410.48_linux.run

echo -e "${GREEN} install 10.0 latest driver ${RESET}"
wget http://113.107.239.242/linchengzhong/myapp/NVIDIA-Linux-x86_64-410.129-diagnostic.run
bash NVIDIA-Linux-x86_64-410.129-diagnostic.run --silent
rm -r NVIDIA-Linux-x86_64-410.129-diagnostic.run

echo -e "${GREEN} install cudnn ${RESET}"
wget http://113.107.239.242/linchengzhong/myapp/cudnn-10.0-linux-x64-v7.6.3.30.tgz
tar -zxvf cudnn-10.0-linux-x64-v7.6.3.30.tgz
cp -rf cuda/lib64/* /usr/local/cuda-10.0/lib64/
cp -rf cuda/include/* /usr/local/cuda-10.0/include/
chmod a+r /usr/local/cuda-10.0/include/cudnn.h /usr/local/cuda-10.0/lib64/libcudnn*
rm -r cuda
rm cudnn-10.0-linux-x64-v7.6.3.30.tgz
echo -e "${GREEN} finish ${RESET}"
```


