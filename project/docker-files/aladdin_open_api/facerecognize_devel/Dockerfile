# 生成镜像名 aladdin-open-api:facerecognize-devel-v4
FROM nvidia-cuda:10.0-cudnn7-devel-ubuntu16.04

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

COPY  /root/requirements.txt  /root

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
    # 安装基础应用
    && apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    && apt-get -y install python3-dev libopenblas-dev python-pip python3-pip cmake && pip3 install --upgrade pip\
    # 安装 s2ssdk
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/s2ssdk/4.0.0/s2ssdk.tar.gz \
    && tar -C /usr/local -xzf s2ssdk.tar.gz \
    # 安装python第三方库 
    && mv -f /root/requirements.txt . \
    && pip3 install -r ./requirements.txt -i https://pypi.douban.com/simple\
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp\
    && rm -f /etc/apt/apt.conf.d/proxy.conf