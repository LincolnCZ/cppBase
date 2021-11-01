link_directories(${FFMPEG_DIR}/lib)
add_executable(tutorial01_d tutorial01.c)
target_include_directories(tutorial01_d PUBLIC ${FFMPEG_DIR}/include)
target_link_libraries(tutorial01_d avformat avcodec swscale avutil m z)