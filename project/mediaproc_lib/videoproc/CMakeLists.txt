set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(VIDEO_PROC_SRC VideoDecoder.cpp ImageEncoder.cpp DecodeProc.cpp)

# 生成静态库
add_library(videoproc STATIC ${VIDEO_PROC_SRC} ${VIDEO_THRIFT_SRC} $<TARGET_OBJECTS:core>)
target_link_libraries(videoproc PRIVATE avcodec avformat avutil swresample)

# 生成二进制
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen VIDEO_THRIFT_SRC)
add_executable(videoproc_server_d main.cpp ${VIDEO_THRIFT_SRC})
target_link_libraries(videoproc_server_d videoproc thrift pthread)