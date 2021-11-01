cmake_minimum_required(VERSION 3.1)
project(cppbase)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")
#set(CMAKE_CXX_FLAGS "-g1 -O0 -DNDEBUG -Wall -gstabs+ -std=c++11") # debug

if(CPPBASE_TEST)
    add_definitions(-DLOGGER_USING_PRINT)
endif()

#-----------------------------------基础模块start---------------------------------
# cppbase_utility
aux_source_directory(src/utility UTILITY_SRC)
add_library(cppbase_utility ${UTILITY_SRC})
target_include_directories(cppbase_utility PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# cppbase_core
aux_source_directory(src/core CORE_SRC)
add_library(cppbase_core ${CORE_SRC})
target_include_directories(cppbase_core PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# cppbase_redistool
link_directories(${PROJECT_SOURCE_DIR}/3rd/hiredis/lib ${PROJECT_SOURCE_DIR}/3rd/event2/lib)
aux_source_directory(src/redistool REDIS_TOOL_SRC)
add_library(cppbase_redistool ${REDIS_TOOL_SRC})
target_include_directories(cppbase_redistool PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> ${PROJECT_SOURCE_DIR}/3rd)
target_link_libraries(cppbase_redistool cppbase_core cppbase_utility hiredis event pthread)

# test
if (CPPBASE_TEST)
#    add_subdirectory(src/utility/test)
    add_subdirectory(src/redistool/test)
endif ()
#-------------------------------------end--------------------------------------

# example
#add_subdirectory(example)

# center_config
#add_subdirectory(project/center_config)

# mediaproc_lib
add_subdirectory(project/mediaproc_lib)

# design pattern
#add_subdirectory(project/design_pattern)

# socket test
#add_subdirectory(project/socket)

# ffmpeg_tutorial
#add_subdirectory(project/ffmpeg_tutorial)
