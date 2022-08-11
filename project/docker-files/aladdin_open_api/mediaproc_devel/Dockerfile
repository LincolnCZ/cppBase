# 生成镜像名 aladdin-open-api:mediaproc-devel-v1
FROM devel:base-v1

RUN apt-get update \
    && mkdir -p /root/temp && cd /root/temp \
    # 安装 thrift-0.11.0
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/thrift/0.11.0/thrift_0.11.0.tar.gz \
    && tar -C /usr/local -xzf thrift_0.11.0.tar.gz \
    # 安装 ffmpeg-3.4.7
    && apt install -y libspeex-dev libfdk-aac-dev \
    && wget http://repo.yy.com/dwbuild/selflib/svr_comm_tech/ffmpeg_3.4.7/fdkaac/ffmpeg.tar.gz \
    && tar -C /usr/local -xzf ffmpeg.tar.gz \
    # 安装 libpng-dev
    && apt install -y libpng-dev \
    # 清理
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /root/temp