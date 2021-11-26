include_directories(/usr/include)

# Find required protobuf package
#find_package(protobuf CONFIG REQUIRED)
find_package(Protobuf)

if(protobuf_VERBOSE)
    message(STATUS "Using Protocol Buffers ${protobuf_VERSION}")
endif()

#add_custom_command(OUTPUT addressbook.pb.cc addressbook.pb.h
#        COMMAND protoc
#        ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/addressbook.proto -I${CMAKE_CURRENT_SOURCE_DIR}
#        DEPENDS addressbook.proto
#        VERBATIM)
#
#set_source_files_properties(addressbook.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
#include_directories(${PROJECT_BINARY_DIR})

#add_library(addressbook_proto addressbook.pb.cc)
#target_link_libraries(addressbook_proto protobuf pthread)
#
#add_executable(protobuf_test_d main.cpp)
#set_target_properties(protobuf_test_d PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
#target_link_libraries(protobuf_test_d  addressbook_proto)

#add_executable(protobuf_test_d main.cpp addressbook.pb.cc)
#target_link_libraries(protobuf_test_d protobuf pthread)

add_executable(protobuf_test_d main.cpp addressbook.pb.cc)
target_include_directories(protobuf_test_d PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_link_libraries(protobuf_test_d ${PROTOBUF_LIBRARIES} pthread)