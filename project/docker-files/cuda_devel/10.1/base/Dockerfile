# 生成镜像名 cuda-devel:10.1-cudnn7-base-v1
FROM nvidia-cuda:10.1-cudnn7-devel-ubuntu16.04

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
# 安装开发需要的包，并安装s2ssdk
    && apt-get update \
    && apt-get install -y curl libuuid1 build-essential uuid-dev openssl libssl-dev \
    && apt-get install -y libboost-all-dev libjsoncpp-dev \
    && apt-get install -y cmake wget unzip vim git \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /root/temp && cd /root/temp \
    # 安装s2s
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/s2ssdk/4.0.0/s2ssdk.tar.gz \
    && tar -C /usr/ -xzf s2ssdk.tar.gz \
    # 清理
    && rm -rf /root/temp \
    && rm -rf /var/lib/apt/lists/*