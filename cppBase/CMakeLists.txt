# cppbase_utility
aux_source_directory(utility UTILITY_SRC)
add_library(cppbase_utility ${UTILITY_SRC})
target_include_directories(cppbase_utility PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cppBase>)

# cppbase_core
aux_source_directory(core CORE_SRC)
add_library(cppbase_core ${CORE_SRC})
target_include_directories(cppbase_core PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cppBase>)

# cppbase_redistool
link_directories(${PROJECT_SOURCE_DIR}/3rd/hiredis/lib ${PROJECT_SOURCE_DIR}/3rd/event2/lib)
aux_source_directory(redistool REDIS_TOOL_SRC)
add_library(cppbase_redistool ${REDIS_TOOL_SRC})
target_include_directories(cppbase_redistool PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/cppBase> ${PROJECT_SOURCE_DIR}/3rd)
target_link_libraries(cppbase_redistool cppbase_core cppbase_utility hiredis event pthread)

# test
if (CPPBASE_TEST)
    add_subdirectory(utility/test)
    add_subdirectory(redistool/test)
endif ()