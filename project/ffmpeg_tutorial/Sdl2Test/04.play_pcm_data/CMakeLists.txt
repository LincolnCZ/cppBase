link_directories(
        ${SDL_DIR}/lib)
add_executable(play_pcm_data_d play_pcm_data.cpp)
target_include_directories(play_pcm_data_d PUBLIC
        ${SDL_DIR}/include
        ${SDL_DIR}/include/SDL2)
target_link_libraries(play_pcm_data_d SDL2)