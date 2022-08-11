ARG IMAGE_NAME
FROM ${IMAGE_NAME}:10.0-base-ubuntu16.04
LABEL maintainer "NVIDIA CORPORATION <cudatools@nvidia.com>"

ENV NCCL_VERSION 2.5.6

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
    && apt-get update && apt-get install -y --no-install-recommends \
    cuda-libraries-$CUDA_PKG_VERSION \
    cuda-nvtx-$CUDA_PKG_VERSION \
    libnccl2=$NCCL_VERSION-1+cuda10.0 && \
    apt-mark hold libnccl2 && \
    rm -rf /var/lib/apt/lists/* && \
    rm -f /etc/apt/apt.conf.d/proxy.conf
