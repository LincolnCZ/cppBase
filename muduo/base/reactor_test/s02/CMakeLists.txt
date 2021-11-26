#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_s02_SRC Channel.cc EventLoop.cc Poller.cc Timer.cc TimerQueue.cc)

add_executable(s02_test1_d test1.cc ${REACTOR_s02_SRC})
target_link_libraries(s02_test1_d muduo_base)

add_executable(s02_test2_d test2.cc ${REACTOR_s02_SRC})
target_link_libraries(s02_test2_d muduo_base)

add_executable(s02_test3_d test3.cc ${REACTOR_s02_SRC})
target_link_libraries(s02_test3_d muduo_base)

add_executable(s02_test4_d test4.cc ${REACTOR_s02_SRC})
target_link_libraries(s02_test4_d muduo_base)