#---习惯用法---
# 说明copy and swap idiom
add_executable(example_copy_swap_idiom_d example_copy_swap_idiom.cpp)

# 自己实现的unique_ptr
add_executable(example_unique_ptr_impl_d example_unique_ptr_impl.cpp)

# 实现shared_ptr
add_executable(example_shared_ptr_impl_d example_shared_ptr_impl.cpp)

# 万能引用 和 完美转发forward
add_executable(example_use_forward_d example_use_forward.cpp)

#---其他---
add_subdirectory(thread_test)