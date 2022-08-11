# 生成镜像名 aladdin-open-api:caffe-devel-v2
FROM cuda-devel:cuda10.0-caffe-v2

# 安装 thrift 0.11.0
RUN mkdir -p /root/temp && cd /root/temp \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local/ -xzf thrift_0.11.0.tar.gz \
    && rm -rf /root/temp