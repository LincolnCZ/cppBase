include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(REACTOR_SRC Acceptor.cc Buffer.cc Channel.cc EventLoop.cc EventLoopThread.cc EventLoopThreadPool.cc
        InetAddress.cc Poller.cc Socket.cc SocketsOps.cc TcpConnection.cc TcpServer.cc Timer.cc TimerQueue.cc)

add_executable(s10_test1_d tests/test1.cc ${REACTOR_SRC})
target_link_libraries(s10_test1_d muduo_base)

add_executable(s10_test2_d tests/test2.cc ${REACTOR_SRC})
target_link_libraries(s10_test2_d muduo_base)

add_executable(s10_test3_d tests/test3.cc ${REACTOR_SRC})
target_link_libraries(s10_test3_d muduo_base)

add_executable(s10_test4_d tests/test4.cc ${REACTOR_SRC})
target_link_libraries(s10_test4_d muduo_base)

add_executable(s10_test5_d tests/test5.cc ${REACTOR_SRC})
target_link_libraries(s10_test5_d muduo_base)

add_executable(s10_test6_d tests/test6.cc ${REACTOR_SRC})
target_link_libraries(s10_test6_d muduo_base)

add_executable(s10_test7_d tests/test7.cc ${REACTOR_SRC})
target_link_libraries(s10_test7_d muduo_base)

add_executable(s10_test8_d tests/test8.cc ${REACTOR_SRC})
target_link_libraries(s10_test8_d muduo_base)

add_executable(s10_test9_d tests/test9.cc ${REACTOR_SRC})
target_link_libraries(s10_test9_d muduo_base)

add_executable(s10_test10_d tests/test10.cc ${REACTOR_SRC})
target_link_libraries(s10_test10_d muduo_base)

add_executable(s10_test11_d tests/test11.cc ${REACTOR_SRC})
target_link_libraries(s10_test11_d muduo_base)