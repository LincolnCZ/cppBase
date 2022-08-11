# 生成镜像名 cuda-devel:cuda10.0-caffe-v2
FROM cuda-devel:10.0-cudnn7-base-v1

# 源代码编译安装 OpenCV 3.4.8 和 caffe 1.0
COPY  /root/install_caffe.sh  /root
COPY  /root/install_with_cuda.sh  /root

RUN apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 编译安装 OpenCV
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv -f /root/install_with_cuda.sh . \
    && bash install_with_cuda.sh \
    && cd /root/temp \
    # 编译安装 Caffe
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/caffe/1.0/caffe.tar.gz \
    && tar -xzf caffe.tar.gz && cd caffe \
    && mv -f /root/install_caffe.sh . \
    && bash install_caffe.sh \
    && cd /root/temp \
    # 清理工作
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp 