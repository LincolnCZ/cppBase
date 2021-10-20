link_directories(
        ${SDL_DIR}/lib)
add_executable(play_pcm_data_d play_pcm_data.cpp)
target_include_directories(play_pcm_data_d PUBLIC
        ${SDL_DIR}/include
        ${SDL_DIR}/include/SDL2)
target_link_libraries(play_pcm_data_d SDL2)

add_executable(paly_pcm_data_by_callback_d paly_pcm_data_by_callback.cpp)
target_include_directories(paly_pcm_data_by_callback_d PUBLIC
        ${SDL_DIR}/include
        ${SDL_DIR}/include/SDL2)
target_link_libraries(paly_pcm_data_by_callback_d SDL2)