include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/muduo)

add_subdirectory(sudoku)
add_subdirectory(simple)
add_subdirectory(filetransfer)
add_subdirectory(asio/chat)
add_subdirectory(asio/tutorial)
add_subdirectory(protobuf)

