link_directories(/usr/local/Cellar/ffmpeg/4.3.1_4/lib)
add_executable(tutorial01_d tutorial01.c)
target_include_directories(tutorial01_d PUBLIC /usr/local/Cellar/ffmpeg/4.3.1_4/include)
target_link_libraries(tutorial01_d avformat avcodec swscale avutil m z)