# 生成镜像名 cuda-devel:10.0-cudnn7-base-v1
FROM nvidia-cuda:10.0-cudnn7-devel-ubuntu16.04

# 安装开发需要的包，并安装s2ssdk
RUN apt-get update \
    && apt-get install -y curl libuuid1 build-essential uuid-dev openssl libssl-dev \
    && apt-get install -y libboost-all-dev libjsoncpp-dev \
    && apt-get install -y cmake wget unzip vim git \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/s2ssdk/4.0.0/s2ssdk.tar.gz \
    && tar -C /usr/ -xzf s2ssdk.tar.gz \
    && rm -rf /root/temp