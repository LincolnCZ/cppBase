媒体内容处理使用的docker构建镜像

## devel

默认的CPU构建镜像。

构建的镜像名：`devel`

- base：安装编译必须的系统包，并安装s2ssdk。

## cuda

nvidia提供的cuda镜像构建的DockerFile，使用ubuntu16.04版本。

原始地址：`https://gitlab.com/nvidia/container-images/cuda/-/tree/master/dist/ubuntu16.04`，对应版本: `d442ff6975fb8310d`。

构建的镜像名：`nvidia-cuda`

官方提供的构建过程，大概分为四个步骤：
1. 构建base镜像
2. 构建runtime镜像，用于运行环境
3. 构建devel镜像，用于编译环境
4. 构建runtime和devel对应的cudnn镜像

官方提供的构建流程如下：

``` bash
#/bin/bash

export IMAGE_NAME="nvidia/cuda"
export CUDA_VERSION="9.0"
export OS="ubuntu16.04"

docker build -t "${IMAGE_NAME}:${CUDA_VERSION}-base-${OS}" "dist/${OS}/${CUDA_VERSION}/base"
docker build -t "${IMAGE_NAME}:${CUDA_VERSION}-runtime-${OS}" --build-arg "IMAGE_NAME=${IMAGE_NAME}" "dist/${OS}/${CUDA_VERSION}/runtime"
docker build -t "${IMAGE_NAME}:${CUDA_VERSION}-devel-${OS}" --build-arg "IMAGE_NAME=${IMAGE_NAME}" "dist/${OS}/${CUDA_VERSION}/devel"
```

## cuda_devel

CUDA 开发环境使用的基础镜像，在CUDA观景镜像基础上对 Caffe,TensorRT,python 等环境进行了打包。

构建的镜像名：`cuda-devel:${cuda_version}-${type}-${version}`

目前包含的环境类型：

 - base：安装编译必须的系统包，并安装s2ssdk
 - caffe：基于base，安装OpenCV 3 和 Caffe
 - tensorRT: 基于base，安装 tensorRT
 - python3.7: 基于base，安装 python3.7

## aladdin_open_api

对外服务使用的镜像，目前也用于运行环境。

构建的镜像名：`aladdin-open-api`

 - service-base: 打包了ffmpeg 3.4.8，thrift-0.11.0。基础镜像
 - mediaproc-devel: 构建mediaproc使用的镜像
 - caffe-devel: Caffe 编译运行环境，用于图片服务
 - tensorrt-devel: tensorRT 编译运行环境，用于图片服务
 - caffe-ssd-devel: caffe-ssd 编译运行环境

## aladdin_service
- base 目录：在cuda10.0 10.1 10.2等基础镜像上安装python3.6 和python3.7
    - python3.6-cuda10.1: 在cuda-devel:10.1-cudnn7-base-v1的基础上，安装python3.6版本
    - python3.7-cuda10.1: 在cuda-devel:10.1-cudnn7-base-v1的基础上，安装python3.7版本
    - python3.6-cuda10.2: 在cuda-devel:10.2-cudnn7-base-v1的基础上，安装python3.6版本
    - python3.7-cuda10.2: 在cuda-devel:10.2-cudnn7-base-v1的基础上，安装python3.7版本
- python3.5_devel: 目录：
    - python3.5-cuda10.0-devel: