# 生成镜像名 cuda-devel:cuda10.2-tensorRT-7-v1
FROM cuda-devel:10.2-cudnn7-base-v2

RUN apt-get update \
    # 安装 python3 并修改pip源
    && apt-get -y install python3-dev libopenblas-dev python3-pip && pip3 install --upgrade pip \
    && mkdir /root/.pip/ \ 
    && ( \
        echo '[global]'; \
        echo 'trusted-host = mirrors.aliyun.com'; \
        echo 'index-url = http://mirrors.aliyun.com/pypi/simple/'; \
    ) > /root/.pip/pip.conf \
    && mkdir -p /root/temp && cd /root/temp \
    # 安装 TensorRT 7，包括 C++ 环境和 python3 环境
    && wget -q https://yyci2020.oss-cn-shenzhen.aliyuncs.com/sdk_management/svr_comm_tech/tensorRT/7.0.0.11-cuda-10.2/TensorRT-7.0.0.11.cuda-10.2.cudnn7.6.tar.gz \
    && tar -C /usr/local -xzf TensorRT-7.0.0.11.cuda-10.2.cudnn7.6.tar.gz \
    && cd /usr/local/TensorRT-7.0.0.11 \
    && pip3 install python/tensorrt-7.0.0.11-cp35-none-linux_x86_64.whl \
    && pip3 install uff/uff-0.6.5-py2.py3-none-any.whl \
    && pip3 install graphsurgeon/graphsurgeon-0.4.1-py2.py3-none-any.whl \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp

ENV LD_LIBRARY_PATH /usr/local/TensorRT-7.0.0.11/lib:$LD_LIBRARY_PATH