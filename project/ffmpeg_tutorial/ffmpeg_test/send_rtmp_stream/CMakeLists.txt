link_directories(${FFMPEG_DIR}/lib)
add_executable(send_rtmp_stream_d send_rtmp_stream.cpp)
target_include_directories(send_rtmp_stream_d PUBLIC ${FFMPEG_DIR}/include)
target_link_libraries(send_rtmp_stream_d avformat avcodec swscale avutil m z)