link_directories(${FFMPEG_DIR}/lib)
link_directories(${SDL_DIR}/lib)

add_executable(ffmpeg_sdl_play_audio_d ffmpeg_sdl_play_audio.cpp)
target_include_directories(ffmpeg_sdl_play_audio_d PUBLIC ${FFMPEG_DIR}/include ${SDL_DIR}/include)
target_link_libraries(ffmpeg_sdl_play_audio_d avformat avcodec swscale swresample avutil m z SDL2)

add_executable(ffmpeg_sdl_play_audio_queue_d ffmpeg_sdl_play_audio_queue.cpp)
target_include_directories(ffmpeg_sdl_play_audio_queue_d PUBLIC ${FFMPEG_DIR}/include ${SDL_DIR}/include)
target_link_libraries(ffmpeg_sdl_play_audio_queue_d avformat avcodec swscale swresample avutil m z SDL2)