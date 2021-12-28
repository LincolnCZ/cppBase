cmake_minimum_required(VERSION 3.1)
project(cppbase)

# 根cmake设置的CMAKE_CXX_FLAGS，如果里层cmake没有设置对应的CMAKE_CXX_FLAGS，则会使用根cmake里设置的
set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")
#set(CMAKE_CXX_FLAGS "-g1 -O0 -DNDEBUG -Wall -gstabs+ -std=c++11") # debug

if (CPPBASE_TEST)
    add_definitions(-DLOGGER_USING_PRINT)
endif ()

#-----------------------------------基础模块---------------------------------
# cppBase
add_subdirectory(cppBase)

# muduo
add_subdirectory(muduo)

#-----------------------------------其他项目----------------------------------
# example
add_subdirectory(example)

# center_config
#add_subdirectory(project/center_config)

# mediaproc_lib
#add_subdirectory(project/mediaproc_lib)

# design pattern
#add_subdirectory(project/design_pattern)

# socket test
#add_subdirectory(project/socket)

# ffmpeg_tutorial
#add_subdirectory(project/ffmpeg_tutorial)

#--------------------------------常用cmake 选项说明---------------------------
# PROJECT_SOURCE_DIR：项目的根目录
# CMAKE_CURRENT_SOURCE_DIR：cmake文件所在的目录