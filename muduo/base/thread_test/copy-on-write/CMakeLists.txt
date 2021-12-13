# 非递归锁存在的死锁情况
add_executable(NonRecursiveMutex_test_d 01.NonRecursiveMutex_test.cc)
target_link_libraries(NonRecursiveMutex_test_d muduo_base)

# 解决非递归锁存在的死锁
add_executable(NonRecursiveMutex_fix_d 01.NonRecursiveMutex_fix.cc)
target_link_libraries(NonRecursiveMutex_fix_d muduo_base)

# 单个线程死锁
add_executable(SelfDeadLock_d 02.SelfDeadLock.cc)
target_link_libraries(SelfDeadLock_d muduo_base)

# 多个线程死锁例子
add_executable(MutualDeadLock_d 02.MutualDeadLock.cc)
target_link_libraries(MutualDeadLock_d muduo_base)

# 解决多个线程死锁 -- 注意目前的方案仍然没有解决Request对象析构的race condition
add_executable(MutualDeadLock_fix_d 02.MutualDeadLock_fix.cc)
target_link_libraries(MutualDeadLock_fix_d muduo_base)

# 解决多个线程死锁
add_executable(MutualDeadLock_fix2_d 02.MutualDeadLock_fix2.cc)
target_link_libraries(MutualDeadLock_fix2_d muduo_base)

# 用普通mutex替换读写锁的一个例子
add_executable(Customer_test_d 03.Customer.cc)
target_link_libraries(Customer_test_d muduo_base)