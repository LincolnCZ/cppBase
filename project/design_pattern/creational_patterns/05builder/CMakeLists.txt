aux_source_directory(. CODE_SRC)
add_executable(builder_pattern_d ${CODE_SRC})
target_link_libraries(builder_pattern_d pthread)