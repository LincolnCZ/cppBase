link_directories(${FFMPEG_DIR}/lib)
link_directories(${SDL_DIR}/lib)

add_executable(ffmpeg_sdl_play_video_d simplest_ffmpeg_player_su.cpp)
target_include_directories(ffmpeg_sdl_play_video_d PUBLIC ${FFMPEG_DIR}/include ${SDL_DIR}/include)
target_link_libraries(ffmpeg_sdl_play_video_d avformat avcodec swscale avutil m z SDL2)