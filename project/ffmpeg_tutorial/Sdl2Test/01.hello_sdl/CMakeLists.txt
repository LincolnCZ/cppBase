link_directories(${SDL_DIR}/lib)
add_executable(hello_sdl_d hello_sdl.cpp)
target_include_directories(hello_sdl_d PUBLIC ${SDL_DIR}/include)
target_link_libraries(hello_sdl_d SDL2 SDL2_test SDL2main)