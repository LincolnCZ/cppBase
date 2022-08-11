# 生成镜像名 aladdin-open-api:onnx-runtime-v1
# 运行onnx的python环境，可以使用tensorrt和onnxruntime
FROM cuda-devel:cuda10.2-tensorRT-7-v1

COPY  /root/requirements.txt  /root
COPY  /root/install_with_cuda.sh  /root

RUN mkdir -p /root/temp && cd /root/temp \
    # 安装python依赖
    && mv -f /root/requirements.txt . \
    && pip3 install -r requirements.txt \
    && cd /root/temp \
    # 编译安装 OpenCV
    && wget -q http://repo.yy.com/dwbuild/selflib/svr_comm_tech/opencv3_src/3.4.8/opencv3.tar.gz \
    && tar -xzf opencv3.tar.gz && cd opencv3 \
    && mv -f /root/install_with_cuda.sh . \
    && bash install_with_cuda.sh \
    # 清理工作
    && rm -rf /root/temp