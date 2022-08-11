#!/bin/bash
set -e

# Caffe base
apt-get install -y libprotobuf-dev libleveldb-dev libsnappy-dev libhdf5-serial-dev libgflags-dev libgoogle-glog-dev liblmdb-dev protobuf-compiler libatlas-base-dev libjsoncpp-dev
apt-get install -y libopenblas-dev liblapacke-dev checkinstall libevent-dev

# build Caffe
mkdir -p build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release \
    -DCPU_ONLY=OFF -DUSE_CUDNN=ON \
    -DCUDA_ARCH_NAME=Manual -DCUDA_ARCH_BIN="61 70 75" \
    -DBUILD_python=OFF -DBUILD_docs=OFF

make -j 8
make install