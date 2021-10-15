set(SDL_DIR /usr/local/Cellar/sdl2/2.0.16)
set(SDL_IMAGE_DIR /usr/local/Cellar/sdl2_image/2.0.5)
set(FFMPEG_DIR /usr/local/Cellar/ffmpeg/4.4_2)

add_subdirectory(01.hello_sdl)
add_subdirectory(02.show_image)
add_subdirectory(03.show_yuv_data)
add_subdirectory(04.play_pcm_data)
add_subdirectory(05.ffmpeg_sdl_play_video)