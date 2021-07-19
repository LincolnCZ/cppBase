aux_source_directory(. CODE_SRC)
add_executable(factory_method_pattern_d ${CODE_SRC})
target_link_libraries(factory_method_pattern_d pthread)