set(CMAKE_CXX_FLAGS "-g1 -O2 -DNDEBUG -Wall -std=c++11")

aux_source_directory(. CODE_SRC)
add_executable(my_smart_pointer_d ${CODE_SRC})