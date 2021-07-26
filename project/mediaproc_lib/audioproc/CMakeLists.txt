set(AUDIO_PROC_SRC AudioProcessor.cpp AudioEncode.cpp ffutil.c)

# 生成静态库
add_library(audioproc STATIC ${AUDIO_PROC_SRC} $<TARGET_OBJECTS:core>)
target_link_libraries(audioproc avcodec avformat swresample avutil pthread)

# 生成二进制
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gencpp AUDIO_THRIFT_SRC)
add_executable(audioproc_server_d main.cpp ${AUDIO_THRIFT_SRC})
target_link_libraries(audioproc_server_d audioproc thrift pthread)