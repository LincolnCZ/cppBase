include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(REACTOR_SRC Acceptor.cc Buffer.cc Channel.cc Connector.cc EPoller.cc EventLoop.cc EventLoopThread.cc
        EventLoopThreadPool.cc InetAddress.cc Poller.cc Socket.cc SocketsOps.cc TcpClient.cc TcpConnection.cc
        TcpServer.cc Timer.cc TimerQueue.cc)

add_executable(s13_test1_d tests/test1.cc ${REACTOR_SRC})
target_link_libraries(s13_test1_d muduo_base)

add_executable(s13_test2_d tests/test2.cc ${REACTOR_SRC})
target_link_libraries(s13_test2_d muduo_base)

add_executable(s13_test3_d tests/test3.cc ${REACTOR_SRC})
target_link_libraries(s13_test3_d muduo_base)

add_executable(s13_test4_d tests/test4.cc ${REACTOR_SRC})
target_link_libraries(s13_test4_d muduo_base)

add_executable(s13_test5_d tests/test5.cc ${REACTOR_SRC})
target_link_libraries(s13_test5_d muduo_base)

add_executable(s13_test6_d tests/test6.cc ${REACTOR_SRC})
target_link_libraries(s13_test6_d muduo_base)

add_executable(s13_test7_d tests/test7.cc ${REACTOR_SRC})
target_link_libraries(s13_test7_d muduo_base)

add_executable(s13_test8_d tests/test8.cc ${REACTOR_SRC})
target_link_libraries(s13_test8_d muduo_base)

add_executable(s13_test9_d tests/test9.cc ${REACTOR_SRC})
target_link_libraries(s13_test9_d muduo_base)

add_executable(s13_test10_d tests/test10.cc ${REACTOR_SRC})
target_link_libraries(s13_test10_d muduo_base)

add_executable(s13_test11_d tests/test11.cc ${REACTOR_SRC})
target_link_libraries(s13_test11_d muduo_base)

add_executable(s13_test12_d tests/test12.cc ${REACTOR_SRC})
target_link_libraries(s13_test12_d muduo_base)

add_executable(s13_test13_d tests/test13.cc ${REACTOR_SRC})
target_link_libraries(s13_test13_d muduo_base)