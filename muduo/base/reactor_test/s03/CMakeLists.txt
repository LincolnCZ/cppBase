#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_s03_SRC Channel.cc EventLoop.cc EventLoopThread.cc Poller.cc Timer.cc TimerQueue.cc)

add_executable(s03_test1_d tests/test1.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test1_d muduo_base)

add_executable(s03_test2_d tests/test2.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test2_d muduo_base)

add_executable(s03_test3_d tests/test3.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test3_d muduo_base)

add_executable(s03_test4_d tests/test4.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test4_d muduo_base)

add_executable(s03_test5_d tests/test5.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test5_d muduo_base)

add_executable(s03_test6_d tests/test6.cc ${REACTOR_s03_SRC})
target_link_libraries(s03_test6_d muduo_base)