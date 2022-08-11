# 生成镜像名 aladdin-open-api:ocr-runtime-v1
FROM cuda-devel:10.1-python3.7-v1

COPY /root/requirments.txt /root/

# PaddleOCR base on Python3.7
RUN mkdir /root/.pip/ \
	&& ( \
        echo '[global]'; \
		echo 'trusted-host = mirrors.aliyun.com'; \
		echo 'index-url = http://mirrors.aliyun.com/pypi/simple/'; \
	) > /root/.pip/pip.conf \
	&& cd /root	\
	&& python3.7 -m pip install --upgrade pip \
	&& python3.7 -m pip install -r requirments.txt \
	&& python3.7 -m pip install paddlepaddle-gpu \
	&& rm -f /root/requirments.txt
