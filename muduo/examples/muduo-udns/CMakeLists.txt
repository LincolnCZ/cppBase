link_directories(${CMAKE_CURRENT_SOURCE_DIR}/udns-0.2)
#aux_source_directory(udns-0.2 UDNS_SRC)

add_executable(udns_test_d dns.cc Resolver.cc)
target_include_directories(udns_test_d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/udns-0.2)
target_link_libraries(udns_test_d muduo_net udns)