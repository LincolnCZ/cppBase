link_directories(
        ${SDL_DIR}/lib
        ${SDL_IMAGE_DIR}/lib)
add_executable(show_image_d show_image.cpp)
target_include_directories(show_image_d PUBLIC
        ${SDL_DIR}/include
        ${SDL_DIR}/include/SDL2
        ${SDL_IMAGE_DIR}/include)
target_link_libraries(show_image_d SDL2 SDL2_image)