# 重新设置CMAKE_CXX_FLAGS，忽略告警
set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")

# 头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/muduo)

# 测试
add_subdirectory(reactor_test)

# 网络库使用相关
add_subdirectory(sudoku)
add_subdirectory(simple)
add_subdirectory(filetransfer)
add_subdirectory(asio/chat)
add_subdirectory(asio/tutorial)
add_subdirectory(protobuf)
add_subdirectory(maxconnection)
add_subdirectory(idleconnection)
add_subdirectory(hub)
add_subdirectory(multiplexer)
add_subdirectory(socks4a)
add_subdirectory(shorturl)
add_subdirectory(muduo-udns)
#add_subdirectory(cdns)
#add_subdirectory(curl)
