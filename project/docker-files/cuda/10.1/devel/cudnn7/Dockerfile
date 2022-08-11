ARG IMAGE_NAME
FROM ${IMAGE_NAME}:10.1-devel-ubuntu16.04
LABEL maintainer "NVIDIA CORPORATION <cudatools@nvidia.com>"

ENV CUDNN_VERSION 7.6.5.32
LABEL com.nvidia.cudnn.version="${CUDNN_VERSION}"

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
    && apt-get update && apt-get install -y --no-install-recommends \
        libcudnn7=$CUDNN_VERSION-1+cuda10.1 \
        libcudnn7-dev=$CUDNN_VERSION-1+cuda10.1 \
    && apt-mark hold libcudnn7 \
    && rm -rf /var/lib/apt/lists/*