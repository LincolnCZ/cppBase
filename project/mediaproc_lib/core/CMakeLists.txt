cmake_minimum_required (VERSION 3.0)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} MEDIA_PROC_CORE_SRC)
add_library(core OBJECT ${MEDIA_PROC_CORE_SRC})