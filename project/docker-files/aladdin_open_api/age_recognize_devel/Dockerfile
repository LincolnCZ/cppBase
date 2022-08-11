# 生成镜像名 aladdin-open-api:age-recognize-devel-v2
FROM cuda-devel:cuda10.0-tensorRT-7-v2

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

COPY  /root/requirements.txt  /root

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
    # 安装python第三方库 
    && mkdir -p /root/temp && cd /root/temp \
    && mv -f /root/requirements.txt . \
    && pip3 install -r ./requirements.txt -i https://pypi.douban.com/simple \
    # 安装 torch2trt
    && git clone https://github.com/NVIDIA-AI-IOT/torch2trt \
    && cd torch2trt \
    && python3 setup.py install \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp\
    && rm -f /etc/apt/apt.conf.d/proxy.conf