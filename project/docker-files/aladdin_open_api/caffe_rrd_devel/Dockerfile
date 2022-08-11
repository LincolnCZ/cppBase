# 生成镜像名 aladdin-open-api:caffe-rrd-devel-v1
FROM cuda-devel:10.0-cudnn7-base-v1

COPY /root/install_caffe.sh  /root
COPY /root/install_opencv.sh  /root

# 首先删除cuda使用的软件源，避免apt update网络异常
RUN rm -f /etc/apt/sources.list.d/* \
    && apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 编译安装 OpenCV
    && wget -q http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv -f /root/install_opencv.sh . \
    && bash install_opencv.sh \
    && cd /root/temp \
    # 编译安装 Caffe-RefineDet，在原始代码基础上。调整了构建文件
    && wget -q https://yyci2020.oss-cn-shenzhen.aliyuncs.com/sdk_management/svr_comm_tech/caffe/Caffe_RefineDet/RefineDet.tar.gz \
    && tar -xzf RefineDet.tar.gz && cd RefineDet \
    && mv -f /root/install_caffe.sh . \
    && bash install_caffe.sh \
    && cd /root/temp \
    # 安装 thrift
    && wget -q http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local/ -xzf thrift_0.11.0.tar.gz \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp 