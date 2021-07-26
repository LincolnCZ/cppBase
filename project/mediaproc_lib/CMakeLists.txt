#cmake_minimum_required(VERSION 3.0)
#project(cpp_media_library)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g1 -O2 -Wall)
if(HAS_EXAMPLE)
    add_definitions(-DLOGGER_USING_PRINT)
else()
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(/usr/local/include)
include_directories(/usr/include/libpng12)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/include)
link_directories(/usr/local/lib)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(core)

add_subdirectory(audioproc)
add_subdirectory(videoproc)
add_subdirectory(imageproc)

# example
if(HAS_EXAMPLE)
    add_executable(example example.cpp)
    target_link_libraries(example audioproc videoproc imageproc avutil)
endif()
