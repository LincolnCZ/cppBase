# 生成镜像名 aladdin-open-api:caffe-ssd-devel-v1
FROM cuda-devel:10.0-cudnn7-base-v1

COPY /root/install_caffe.sh  /root
COPY /root/install_opencv.sh  /root

RUN apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 编译安装 OpenCV
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv -f /root/install_opencv.sh . \
    && bash install_opencv.sh \
    && cd /root/temp \
    # 编译安装 Caffe-ssd
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/caffe/caffe_ssd/caffe-ssd.tar.gz \
    && tar -xzf caffe-ssd.tar.gz \
    && mv -f /root/install_caffe.sh . \
    && bash install_caffe.sh \
    && cd /root/temp \
    # 安装 thrift
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local/ -xzf thrift_0.11.0.tar.gz \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp 