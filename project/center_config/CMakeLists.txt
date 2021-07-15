aux_source_directory(src CODE_SRC)

add_executable(center_config_d ${CODE_SRC})
target_include_directories(center_config_d PUBLIC src /usr/lib/x86_64-linux-gnu)
target_link_libraries(center_config_d jsoncpp)


