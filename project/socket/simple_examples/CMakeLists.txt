# Macro for add example target
macro(add_socket_target EXAMPLE_TARGET)
    add_executable("example_${EXAMPLE_TARGET}" "example_${EXAMPLE_TARGET}.cpp")
    target_link_libraries("example_${EXAMPLE_TARGET}" pthread crypto)
endmacro(add_socket_target)

add_socket_target(getaddrinfo)
add_socket_target(tcpdump_impl)