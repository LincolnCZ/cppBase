include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(REACTOR_s01_SRC Channel.cc EventLoop.cc Poller.cc)

add_executable(s01_test1_d test1.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test1_d muduo_base)

add_executable(s01_test2_d test2.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test2_d muduo_base)

add_executable(s01_test3_d test3.cc ${REACTOR_s01_SRC})
target_link_libraries(s01_test3_d muduo_base)