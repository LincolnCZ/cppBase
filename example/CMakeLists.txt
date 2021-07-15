# Macro for add example target
macro(add_example_target EXAMPLE_TARGET)
    add_executable("example_${EXAMPLE_TARGET}" "example_${EXAMPLE_TARGET}.cpp")
    target_link_libraries("example_${EXAMPLE_TARGET}" pthread crypto)
endmacro(add_example_target)

add_example_target(hash)
add_example_target(hash_2_file)
add_example_target(for_range)
add_example_target(std_bind)
add_example_target(std_function)
add_example_target(file)
add_example_target(md5)
add_example_target(test)

#stl
add_subdirectory(stl)
add_subdirectory(batchWork)