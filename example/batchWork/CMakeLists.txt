set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")


aux_source_directory(. CODE_SRC)

add_executable(batch_worker_d ${CODE_SRC})
target_link_libraries(batch_worker_d pthread)