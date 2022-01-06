#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_s00_SRC EventLoop.cc)

add_executable(s00_test1_d test1.cc ${REACTOR_s00_SRC})
target_link_libraries(s00_test1_d muduo_base)

add_executable(s00_test2_d test2.cc ${REACTOR_SRC})
target_link_libraries(s00_test2_d muduo_base)