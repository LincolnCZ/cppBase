set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")

# Macro for add example target
macro(add_stl_example_target STL_EXAMPLE_TARGET)
    add_executable("example_${STL_EXAMPLE_TARGET}" "example_${STL_EXAMPLE_TARGET}.cpp")
        target_link_libraries("example_${STL_EXAMPLE_TARGET}" pthread )
endmacro(add_stl_example_target)

add_stl_example_target(queue)
add_stl_example_target(promise_future)
add_stl_example_target(smart_ptr)
add_stl_example_target(stl_test)