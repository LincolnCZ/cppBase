aux_source_directory(. CODE_SRC)
add_executable(abstract_factory_pattern_d ${CODE_SRC})
target_link_libraries(abstract_factory_pattern_d pthread)