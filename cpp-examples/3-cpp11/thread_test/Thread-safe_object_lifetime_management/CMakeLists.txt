# Counter示例--单线程安全--多线程时有问题
add_executable(Counter_d 01.Counter.cc)
target_link_libraries(Counter_d muduo_base)

# 非线程安全版本的observe模式
add_executable(Observer_d 02.Observer.cc)
target_link_libraries(Observer_d muduo_base)

# 不是100%线程安全的observe模式
add_executable(Observer_safe_d 02.Observer_safe.cc)
target_link_libraries(Observer_safe_d muduo_base)

# 对象池
add_executable(Factory_d 03.Factory.cc)
target_link_libraries(Factory_d muduo_base)

#
add_executable(SignalSlot_test_d 04.SignalSlot_test.cpp)
target_link_libraries(SignalSlot_test_d muduo_base)

# 通用的弱回调封装
#add_executable(WeakCallback_d 05.WeakCallback.h)
#target_link_libraries(WeakCallback_d muduo_base)