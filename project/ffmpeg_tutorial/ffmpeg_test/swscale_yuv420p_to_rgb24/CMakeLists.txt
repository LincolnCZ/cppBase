link_directories(${FFMPEG_DIR}/lib)
add_executable(swscale_yuv420p_to_rgb24_d swscale_yuv420p_to_rgb24.cpp)
target_include_directories(swscale_yuv420p_to_rgb24_d PUBLIC ${FFMPEG_DIR}/include)
target_link_libraries(swscale_yuv420p_to_rgb24_d avformat avcodec swscale avutil m z)