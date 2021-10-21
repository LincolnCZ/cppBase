link_directories(${FFMPEG_DIR}/lib)
link_directories(${SDL_DIR}/lib)

add_executable(tutorial04_d tutorial04.cpp)
target_include_directories(tutorial04_d PUBLIC ${SDL_DIR}/include/SDL2 ${SDL_DIR}/include ${FFMPEG_DIR}/include)
target_link_libraries(tutorial04_d avformat avcodec swscale avutil m z SDL2 swresample)