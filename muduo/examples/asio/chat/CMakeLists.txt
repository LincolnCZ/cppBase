# 客户端
add_executable(asio_chat_client client.cc)
target_link_libraries(asio_chat_client muduo_net)

# 服务器端
add_executable(asio_chat_server server.cc)
target_link_libraries(asio_chat_server muduo_net)

add_executable(asio_chat_loadtest loadtest.cc)
target_link_libraries(asio_chat_loadtest muduo_net)

# 使用多线程TcpServer，并用mutex来保护共享数据。
add_executable(asio_chat_server_threaded server_threaded.cc)

# 对共享数据以§2.8“借shared_ptr实现copy-on-write”的手法来降低锁竞争。
add_executable(asio_chat_server_threaded_efficient server_threaded_efficient.cc)
target_link_libraries(asio_chat_server_threaded_efficient muduo_net)

# 采用thread local变量，实现多线程高效转发，这个例子值得仔细阅读理解。
add_executable(asio_chat_server_threaded_highperformance server_threaded_highperformance.cc)
target_link_libraries(asio_chat_server_threaded_highperformance muduo_net)

