aux_source_directory(. CODE_SRC)
add_executable(simple_factory_pattern_d ${CODE_SRC})
target_link_libraries(simple_factory_pattern_d pthread)