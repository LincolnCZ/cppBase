# 生成镜像名 aladdin-service:python3.6-cuda10.0-devel-v3
FROM aladdin-service:python3.6-cuda10.0-v1

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

COPY  /root/requirements.txt  /root

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
    # 安装基础应用
    # && apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 安装python第三方库 
    && mv -f /root/requirements.txt . \
    && python3.6 -m pip install -r ./requirements.txt -i https://pypi.douban.com/simple\
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp\
    && rm -f /etc/apt/apt.conf.d/proxy.conf