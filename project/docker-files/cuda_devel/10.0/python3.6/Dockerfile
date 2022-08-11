# 生成镜像名 cuda-devel:10.0-python3.6-v1
FROM cuda-devel:10.0-cudnn7-base-v1

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
      echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
      echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
    ) > /etc/apt/apt.conf.d/proxy.conf \
    && apt-get update \
    # 安装python3.6
    && apt-get install -y zlibc libssl-dev libreadline-dev libffi-dev zlib1g-dev libsqlite3-dev \
    && apt-get install -y lzma lzma-dev libgdbm-dev libbz2-dev libncurses5-dev \
    && mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/Python-3.6.8.tgz/v1/Python-3.6.8.tgz \
    && tar -xzvf Python-3.6.8.tgz \
    && cd Python-3.6.8 \
    && ./configure --prefix=/usr/local/python3.6 \ 
	  && make -j 8 && make install \
	  && ln -s /usr/local/python3.6/bin/python3.6 /usr/bin/python3.6 \
    # 清理工作
    && rm -rf /root/temp \
    && rm -rf /var/lib/apt/lists/* \
    && rm -f /etc/apt/apt.conf.d/proxy.conf
