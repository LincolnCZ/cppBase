ARG IMAGE_NAME
FROM ${IMAGE_NAME}:10.1-runtime-ubuntu16.04
LABEL maintainer "NVIDIA CORPORATION <cudatools@nvidia.com>"

# 运维要求增加apt代理，解决翻墙问题
RUN ( \
  echo 'Acquire::http::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
  echo 'Acquire::https::Proxy "http://hk-proxy.bp.yy.com:8899/";'; \
) > /etc/apt/apt.conf.d/proxy.conf \
        && apt-get update && apt-get install -y --no-install-recommends \
        cuda-nvml-dev-$CUDA_PKG_VERSION \
        cuda-command-line-tools-$CUDA_PKG_VERSION \
        cuda-libraries-dev-$CUDA_PKG_VERSION \
        cuda-minimal-build-$CUDA_PKG_VERSION \
        libnccl-dev=$NCCL_VERSION-1+cuda10.1 \
        libcublas-dev=10.2.1.243-1 \
    && rm -rf /var/lib/apt/lists/* \
    && rm -f /etc/apt/apt.conf.d/proxy.conf

ENV LIBRARY_PATH /usr/local/cuda/lib64/stubs