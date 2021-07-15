enable_testing()

link_directories(${PROJECT_SOURCE_DIR}/3rd/googletest/lib)
include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(. UTILITY_TEST)
add_executable(cppbase_utility_test main.cpp ${UTILITY_TEST})
target_include_directories(cppbase_utility_test PUBLIC ${PROJECT_SOURCE_DIR}/3rd/googletest/include)
target_link_libraries(cppbase_utility_test cppbase_utility gmock gmock_main gtest gtest_main pthread) # pthread 需要放到最后
