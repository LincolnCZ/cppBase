# 生成镜像名 devel:base-v1
FROM ubuntu_16.04:latest

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

RUN apt-get update \
    && apt-get install -y curl libuuid1 build-essential uuid-dev openssl libssl-dev \
    && apt-get install -y libboost-all-dev libjsoncpp-dev \
    && apt-get install -y cmake wget unzip vim git \
    # 安装 s2ssdk
    && mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/s2ssdk/4.0.0/s2ssdk.tar.gz \
    && tar -C /usr/local -xzf s2ssdk.tar.gz \
    # 清理
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp