#!/bin/bash

#从包发布系统触发构建时,会把目标进程字符串传到构建脚本，可根据实际情况传递此参数到build.sh中
TARGET_PROCESS="$1"

WORKSPACE=$(pwd)
DOCKER_IMAGE="registry.cn-hangzhou.aliyuncs.com/yy_default/aladdin-open-api:mediaproc-devel-v1"

#拉取最新镜像
docker pull $DOCKER_IMAGE

#运行镜像，并把本地当前目录映射到容器，容器启动后进入工作目录，并执行build.sh进行编译。
docker run $RUN_USER --rm -v $WORKSPACE:$WORKSPACE $DOCKER_IMAGE sh -c "cd $WORKSPACE && bash build.sh $TARGET_PROCESS"