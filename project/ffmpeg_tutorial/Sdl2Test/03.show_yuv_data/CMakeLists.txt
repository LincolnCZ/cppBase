link_directories(
        ${SDL_DIR}/lib)
add_executable(show_yuv_data_d show_yuv_data.cpp)
target_include_directories(show_yuv_data_d PUBLIC
        ${SDL_DIR}/include
        ${SDL_DIR}/include/SDL2)
target_link_libraries(show_yuv_data_d SDL2)