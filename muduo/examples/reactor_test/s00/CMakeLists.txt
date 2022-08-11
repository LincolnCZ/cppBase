#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_s00_SRC EventLoop.cc)

#  test1.cc 会在主线程和子线程分别创建一个 EventLoop，程序正常运行退出。
add_executable(s00_test1_d test1.cc ${REACTOR_s00_SRC})
target_link_libraries(s00_test1_d muduo_base)

# test2.cc 是个负面测试，它在主线程创建了 EventLoop 对象，却试图在另一个线程调用其 EventLoop::loop()，程序会因断言失效而异常终止。
add_executable(s00_test2_d test2.cc ${REACTOR_SRC})
target_link_libraries(s00_test2_d muduo_base)