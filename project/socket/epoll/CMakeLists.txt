SET(EPOLL_SERVER epoll_server.cpp)
add_executable(epoll_server_d ${EPOLL_SERVER})

add_executable(epoll_client_d epoll_client.cpp)

SET(SIMPLE_CLIENT simple_client.cpp)
add_executable(simple_client_d ${SIMPLE_CLIENT})

