aux_source_directory(. BASIC_SRC)
add_executable(basic_test_d ${BASIC_SRC})
#target_include_directories(tutorial01_d PUBLIC ${FFMPEG_DIR}/include)
#target_link_libraries(tutorial01_d avformat avcodec swscale avutil m z)