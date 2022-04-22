message(STATUS "examples_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})

# Macro for add example target
macro(add_example_target EXAMPLE_TARGET)
    add_executable("example_${EXAMPLE_TARGET}" "example_${EXAMPLE_TARGET}.cpp")
    target_link_libraries("example_${EXAMPLE_TARGET}" pthread)
endmacro(add_example_target)

# test
add_example_target(for_range)
add_example_target(test)
add_example_target(test2)
add_example_target(test3)

# md5
add_executable(example_md5_d example_md5.cpp)
target_link_libraries(example_md5_d crypto)

# stl
add_subdirectory(stl)

# yy
add_subdirectory(yy)

# 其他测试项目
add_subdirectory(batchWork)
#add_subdirectory(smart_pointer)

# protobuf
#if (PROTOBUF_FOUND)
#    add_subdirectory(protobuf_test)
#endif ()
add_subdirectory(protobuf_test)