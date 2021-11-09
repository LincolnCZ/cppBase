link_directories(${FFMPEG_DIR}/lib)
add_executable(yuv_encode_2_h.264_d yuv_encode_2_h.264.cpp)
target_include_directories(yuv_encode_2_h.264_d PUBLIC ${FFMPEG_DIR}/include)
target_link_libraries(yuv_encode_2_h.264_d avformat avcodec swscale avutil m z)