# 生成镜像名 cuda-devel:10.2-cudnn7-base-v2
FROM nvidia-cuda:10.2-cudnn7-devel-ubuntu16.04

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

# 首先删除cuda使用的软件源，避免apt update网络异常
RUN rm -f /etc/apt/sources.list.d/* \
    # 安装开发需要的包
    && apt-get update \
    && apt-get install -y curl libuuid1 build-essential uuid-dev openssl libssl-dev \
    && apt-get install -y libboost-all-dev libjsoncpp-dev libtinyxml2-dev libevent-dev \
    && apt-get install -y cmake wget unzip vim git \
    # 安装 s2ssdk
    && mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/s2ssdk/4.0.0/s2ssdk.tar.gz \
    && tar -C /usr/local -xzf s2ssdk.tar.gz \
    # 清理
    && rm -rf /root/temp \
    && rm -rf /var/lib/apt/lists/*