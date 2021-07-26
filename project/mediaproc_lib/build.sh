#!/bin/bash

#########################
#  自动构建的脚本
#
echo "------------------ enter buidling ---------------------------"
workpath=$(pwd)
echo "build path ${workpath}"

echo "------------------ start buidling ---------------------------"
#编译程序
mkdir -p build && cd build
cmake ..
if [ $? -ne 0 ];then exit 255;fi
make
if [ $? -ne 0 ];then exit 255;fi
cd ${workpath}

#按构建系统约定规则输出
mkdir -p ./output/include/audioproc
mkdir -p ./output/bin
mkdir -p ./output/lib

# audioproc
cp audioproc/AudioProcessor.h ./output/include/audioproc/
cp build/audioproc/libaudioproc.a ./output/lib/
cp build/audioproc/audioproc_server_d ./output/bin/
# videoproc
cp build/videoproc/videoproc_server_d ./output/bin/
# imageproc
cp build/imageproc/imageproc_server_d ./output/bin/