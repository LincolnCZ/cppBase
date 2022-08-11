include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(REACTOR_s01_SRC Channel.cc EventLoop.cc Poller.cc)

add_executable(s01_test1_d test1.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test1_d muduo_base)

add_executable(s01_test2_d test2.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test2_d muduo_base)

# 用 timerfd 实现了一个单次触发的定时器
# 这个程序利用 Channel 将 timerfd 的 readable 事件转发给 timerout() 函数。
add_executable(s01_test3_d test3.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test3_d muduo_base)