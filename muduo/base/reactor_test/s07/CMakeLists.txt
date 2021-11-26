#include_directories(${PROJECT_SOURCE_DIR})

SET(REACTOR_SRC Acceptor.cc Buffer.cc Channel.cc EventLoop.cc EventLoopThread.cc InetAddress.cc Poller.cc
        Socket.cc SocketsOps.cc TcpConnection.cc TcpServer.cc Timer.cc TimerQueue.cc)

add_executable(s07_test1_d tests/test1.cc ${REACTOR_SRC})
target_link_libraries(s07_test1_d muduo_base)

add_executable(s07_test2_d tests/test2.cc ${REACTOR_SRC})
target_link_libraries(s07_test2_d muduo_base)

add_executable(s07_test3_d tests/test3.cc ${REACTOR_SRC})
target_link_libraries(s07_test3_d muduo_base)

add_executable(s07_test4_d tests/test4.cc ${REACTOR_SRC})
target_link_libraries(s07_test4_d muduo_base)

add_executable(s07_test5_d tests/test5.cc ${REACTOR_SRC})
target_link_libraries(s07_test5_d muduo_base)

add_executable(s07_test6_d tests/test6.cc ${REACTOR_SRC})
target_link_libraries(s07_test6_d muduo_base)

add_executable(s07_test7_d tests/test7.cc ${REACTOR_SRC})
target_link_libraries(s07_test7_d muduo_base)

add_executable(s07_test8_d tests/test8.cc ${REACTOR_SRC})
target_link_libraries(s07_test8_d muduo_base)