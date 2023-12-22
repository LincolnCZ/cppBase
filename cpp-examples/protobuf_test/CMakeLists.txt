include_directories(/usr/include)

#set(PROTOBUF_DIR ${PROJECT_SOURCE_DIR}/3rd/protobuf-3.19.0)
set(PROTOBUF_DIR /usr/local/Cellar/protobuf/21.11)

if(protobuf_VERBOSE)
    message(STATUS "Using Protocol Buffers ${protobuf_VERSION}")
endif()

## -----------使用命令在构建的时候生成对应pb文件--------------
#add_custom_command(OUTPUT addressbook.pb.cc addressbook.pb.h
#        COMMAND ${PROTOBUF_DIR}/bin/protoc
#        ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/addressbook.proto -I${CMAKE_CURRENT_SOURCE_DIR}
#        DEPENDS addressbook.proto
#        VERBATIM)
#
#set_source_files_properties(addressbook.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
#include_directories(${PROJECT_BINARY_DIR})
#
#link_directories(${PROTOBUF_DIR}/lib)
#add_library(addressbook_proto addressbook.pb.cc)
#target_include_directories(addressbook_proto PUBLIC ${PROTOBUF_DIR}/include)
#target_link_libraries(addressbook_proto protobuf pthread)
#
#add_executable(protobuf_test_d main.cpp)
#set_target_properties(protobuf_test_d PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
#target_link_libraries(protobuf_test_d  addressbook_proto)
## ------------end---------------------------------------

link_directories(${PROTOBUF_DIR}/lib)
add_executable(protobuf_test_d main.cpp addressbook.pb.cc)
target_include_directories(protobuf_test_d PUBLIC ${PROTOBUF_DIR}/include)
target_link_libraries(protobuf_test_d protobuf pthread)