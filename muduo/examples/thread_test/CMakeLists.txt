include_directories(${PROJECT_SOURCE_DIR})

# 线程安全的对象生命期管理
add_subdirectory(Thread-safe_object_lifetime_management)

# 线程同步精要
add_subdirectory(copy-on-write)