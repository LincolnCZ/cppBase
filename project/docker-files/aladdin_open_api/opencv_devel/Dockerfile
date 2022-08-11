# 生成镜像名 aladdin-open-api:opencv-devel-v1
# CPU运行opencv的环境
FROM devel:base-v1

COPY /root/install_opencv.sh /root

RUN mkdir -p /root/temp && cd /root/temp \
    # 安装 thrift-0.11.0
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local -xzf thrift_0.11.0.tar.gz \
    # 编译安装 OpenCV
    && wget -q http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv -f /root/install_opencv.sh . \
    && bash install_opencv.sh \
    # 清理工作
    && rm -rf /root/temp