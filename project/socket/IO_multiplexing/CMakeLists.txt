set(CLIENT_SELECT tcpcliselect.cpp)
add_executable(client_select_d ${CLIENT_SELECT})

set(CLIENT_SELECT_BAD tcpcliselectbad.cpp)
add_executable(client_select_bad_d ${CLIENT_SELECT_BAD})

set(SERVER_SELECT tcpserselect.cpp)
add_executable(server_select_d ${SERVER_SELECT})