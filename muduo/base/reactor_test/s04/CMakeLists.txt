#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_SRC Acceptor.cc Channel.cc EventLoop.cc EventLoopThread.cc InetAddress.cc Poller.cc
        Socket.cc SocketsOps.cc Timer.cc TimerQueue.cc)

add_executable(s04_test1_d test1.cc ${REACTOR_SRC})
target_link_libraries(s04_test1_d muduo_base)

add_executable(s04_test2_d test2.cc ${REACTOR_SRC})
target_link_libraries(s04_test2_d muduo_base)

add_executable(s04_test3_d test3.cc ${REACTOR_SRC})
target_link_libraries(s04_test3_d muduo_base)

add_executable(s04_test4_d test4.cc ${REACTOR_SRC})
target_link_libraries(s04_test4_d muduo_base)

add_executable(s04_test5_d test5.cc ${REACTOR_SRC})
target_link_libraries(s04_test5_d muduo_base)

add_executable(s04_test6_d test6.cc ${REACTOR_SRC})
target_link_libraries(s04_test6_d muduo_base)

add_executable(s04_test7_d test7.cc ${REACTOR_SRC})
target_link_libraries(s04_test7_d muduo_base)