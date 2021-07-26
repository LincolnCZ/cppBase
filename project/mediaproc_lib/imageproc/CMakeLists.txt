set(IMAGE_PROC_SRC image_enc.c anim_util.c imageproc.cpp)

# 生成静态库
add_library(imageproc STATIC ${IMAGE_PROC_SRC} $<TARGET_OBJECTS:core>)
target_link_libraries(imageproc png webp webpdemux avcodec avformat avutil pthread)

# 生成二进制
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gencpp IMAGE_THRIFT_SRC)
add_executable(imageproc_server_d  main.cpp  ${IMAGE_THRIFT_SRC} )
target_link_libraries(imageproc_server_d imageproc thrift pthread)