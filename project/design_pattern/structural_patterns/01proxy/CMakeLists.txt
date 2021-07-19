aux_source_directory(. CODE_SRC)
add_executable(proxy_pattern_d ${CODE_SRC})
target_link_libraries(proxy_pattern_d pthread)