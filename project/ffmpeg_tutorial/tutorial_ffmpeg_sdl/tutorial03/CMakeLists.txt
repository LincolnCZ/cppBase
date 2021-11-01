link_directories(${FFMPEG_DIR}/lib)
link_directories(${SDL_DIR}/lib)

add_executable(tutorial03_d tutorial03.cpp)
target_include_directories(tutorial03_d PUBLIC ${SDL_DIR}/include/SDL2 ${SDL_DIR}/include ${FFMPEG_DIR}/include)
target_link_libraries(tutorial03_d avformat avcodec swscale avutil m z SDL2 swresample)

add_executable(tutorial03_bak_d tutorial03_bak.cpp)
target_include_directories(tutorial03_bak_d PUBLIC ${SDL_DIR}/include/SDL2 ${SDL_DIR}/include ${FFMPEG_DIR}/include)
target_link_libraries(tutorial03_bak_d avformat avcodec swscale avutil m z SDL2 swresample)