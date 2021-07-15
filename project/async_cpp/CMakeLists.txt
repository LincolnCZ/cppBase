cmake_minimum_required(VERSION 3.4)
project(aladdin_cover_check_cpp_d)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -I/usr/include/jsoncpp -D_GLIBCXX_USE_CXX11_ABI=0  -lpthread -pthread -lalibabacloud-oss-cpp-sdk") # -std=gnu++10

set(CMAKE_CXX_STANDARD 11)
set(LIBS_DIR "/data/picture_to_cartoon_libs/")

# google sdk
find_package(storage_client REQUIRED)

set(OpenCV_DIR ${LIBS_DIR}/opencv-3.4.6/build)
set(Torch_DIR ${LIBS_DIR}/libtorch/share/cmake/Torch)
find_package(OpenCV REQUIRED)
find_package(Torch REQUIRED)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# kafka
link_libraries(${CMAKE_SOURCE_DIR}/kafka/lib/librdkafka++.so)
include_directories(${CMAKE_SOURCE_DIR}/kafka/include)
include_directories(${CMAKE_SOURCE_DIR})

link_directories(lib)
link_libraries(/usr/lib/x86_64-linux-gnu/libuuid.a)
link_libraries(/usr/lib/x86_64-linux-gnu/libcurl.so)
link_libraries(${CMAKE_SOURCE_DIR}/json/libjsoncpp.so)
link_libraries(${CMAKE_SOURCE_DIR}/rabbitmq/lib/librabbitmq.a)
link_libraries(/usr/local/lib/libalibabacloud-oss-cpp-sdk.so)
link_libraries(${CMAKE_SOURCE_DIR}/ailib/libcover_qc_models_api.so)

# ai denpend lib
# trt
set(TRTDIR ${LIBS_DIR}/trt7)
set(trtlib ${TRTDIR}/lib/libnvinfer.so ${TRTDIR}/lib/libnvonnxparser.so ${TRTDIR}/lib/libnvinfer_plugin.so ${TRTDIR}/lib/libnvparsers.so)


include_directories(rabbitmq/include)
include_directories(include)
include_directories(.)
include_directories(ailib)

add_executable(aladdin_cover_check_cpp_d main.cpp config.cpp rabbitmqclient.cpp taskparams.cpp restclient/restclient.cc
        restclient/helpers.cc restclient/connection.cc logger.cpp syslog-nb.c include/MetricsEx.cpp ossclient.cpp googleclient.cpp
        staticservice.cpp pushservice.cpp Bs2Client.cpp kafkaclient.cpp)
target_link_libraries(aladdin_cover_check_cpp_d  storage_client ${OpenCV_LIBS} ${TORCH_LIBRARIES} ${trtlib}  -lssl -lcrypto -lcrc32c -lz -lmetrics -lGL -lGLU EGL dl)
