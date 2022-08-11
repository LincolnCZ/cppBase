# 生成镜像名 aladdin-open-api:pytorch-runtime-v1
FROM cuda-devel:cuda10.0-tensorRT-7-v2

COPY  /root/requirements.txt  /root

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
      echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
      echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
    ) > /etc/apt/apt.conf.d/proxy.conf \
    && apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 安装python依赖
    && mv -f /root/requirements.txt . \
    && pip3 install -r ./requirements.txt \
    # 手动安装 pycuda
    && wget -q http://repo.yy.com/dwbuild/selflib/svr_comm_tech/pycuda/2019.1.2/pycuda-2019.1.2.tar.gz \
    && tar -xzf pycuda-2019.1.2.tar.gz && cd pycuda-2019.1.2 \
    && python3 configure.py --cuda-root=/usr/local/cuda \
    && make install \
    && cd /root/temp \
    # 手动安装 torch2trt
    && git clone https://github.com/NVIDIA-AI-IOT/torch2trt \
    && cd torch2trt && python3 setup.py install \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp \
    && rm -f /etc/apt/apt.conf.d/proxy.conf