# 生成镜像名 aladdin-service:python3.7-cuda10.1-v1
FROM cuda-devel:10.1-cudnn7-base-v1

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
      echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
      echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
    ) > /etc/apt/apt.conf.d/proxy.conf \
    && apt-get update \
    # 安装python3.6
    && mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/Python-3.7.6.tgz/v1/Python-3.7.6.tgz \
    && tar -xzvf Python-3.7.6.tgz \
    && cd Python-3.7.6 \
    && ./configure --prefix=/usr/local/python3.7 \ 
	&& make -j 8 && make install \
	&& ln -s /usr/local/python3.7/bin/python3.7 /usr/bin/python3.7 \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp \
    && rm -f /etc/apt/apt.conf.d/proxy.conf

# python3.7使用pip 安装包时：
# python3.7 -m pip install ... 