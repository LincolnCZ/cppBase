# 生成镜像名 aladdin-open-api:tensorrt-devel-v2
FROM cuda-devel:cuda10.0-tensorRT-7-v1

COPY /root/install_with_cuda.sh /root/install_with_cuda.sh

RUN mkdir -p /root/temp && cd /root/temp \
    # 编译安装 OpenCV
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv /root/install_with_cuda.sh . \
    && bash install_with_cuda.sh \
    && cd /root/temp \
    # 安装 thrift 0.11.0
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local/ -xzf thrift_0.11.0.tar.gz \
    && cd /root/temp \
    && rm -rf /root/temp