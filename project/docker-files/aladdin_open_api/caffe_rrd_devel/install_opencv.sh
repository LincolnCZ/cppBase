#!/bin/bash

mkdir -p opencv-3.4.8/build
cd opencv-3.4.8/build

cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DWITH_CUDA=ON -DCUDA_ARCH_BIN="61 70 75" -DCUDA_nppi_LIBRARY=true -DBUILD_opencv_cudacodec=OFF \
    -DCUDA_NVCC_FLAGS="-O3" -DCUDA_FAST_MATH=ON \
    -DWITH_PNG=ON -DBUILD_PNG=OFF \
    -DWITH_JPEG=ON -DBUILD_JPEG=OFF \
    -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DWITH_FFMPEG=OFF -DWITH_GTK=OFF \
    -DWITH_OPENCL=OFF -DWITH_QT=OFF -DWITH_V4L=OFF -DWITH_JASPER=OFF \
    -DWITH_1394=OFF -DWITH_TIFF=OFF -DWITH_OPENEXR=OFF -DWITH_IPP=OFF -DWITH_WEBP=OFF \
    -DBUILD_opencv_superres=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_python2=OFF \
    -DBUILD_opencv_videostab=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_flann=OFF \
    -DBUILD_opencv_ml=OFF -DBUILD_opencv_photo=OFF -DBUILD_opencv_shape=OFF \
    -DBUILD_opencv_cudabgsegm=OFF -DBUILD_opencv_cudaoptflow=OFF -DBUILD_opencv_cudalegacy=OFF

make -j 8
make install