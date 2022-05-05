//smartpoint.h文件
#pragma once

#include <iostream>

class Object {
public:
    int a;
    int b;
};

class SmartPointer;

//使用计数类   这个类的所有成员均设置为private，并将SmartPointer类设为友元
class Counter {
private:
    friend class SmartPointer;

    Counter() : ptr(nullptr), cnt(0) {}

    explicit Counter(Object *p) : ptr(p), cnt(1) {}

    ~Counter() {
        delete ptr;
        std::cout << "call Counter destructor" << std::endl;
    }

    Object *ptr;
    int cnt;
};

class SmartPointer {
public:
    //注意此处构造函数的参数是Object* p，每次创建类的新对象时，初始化指针并将使用次数置为1
    explicit SmartPointer(Object *p) : ptr_counter(new Counter(p)) {
    }

    //当对象作为另一对象的副本而创建时，复制构造函数复制指针并增加相应的使用计数的值
    SmartPointer(const SmartPointer &rhs) : ptr_counter(rhs.ptr_counter) {
        ++ptr_counter->cnt;
    }

    //当对一个对象进行赋值时，赋值操作符减少左操作数所指对象的使用计数的值（如果使用计数减至0，则删除对象），
    //并增加右操作数所指对象使用计数的值。
    SmartPointer &operator=(const SmartPointer &rhs) {
        ++rhs.ptr_counter->cnt;
        --ptr_counter->cnt;
        if (ptr_counter->cnt == 0) {
            delete ptr_counter;
        }
        ptr_counter = rhs.ptr_counter;
        return *this;
    }

    //使用计数为0，则删除基础对象
    ~SmartPointer() {
        std::cout << "call SmartPointer destructor cnt is : " << ptr_counter->cnt << std::endl;
        --ptr_counter->cnt;
        if (ptr_counter->cnt == 0) {
            delete ptr_counter;
        }
    }

    //获取智能指针所包装的指针
    Object *GetPtr() {
        return ptr_counter->ptr;
    }

    Object &GetObject() {
        return *(ptr_counter->ptr);
    }

    Object *operator->() {
        return ptr_counter->ptr;
    }

    Object &operator*() {
        return *(ptr_counter->ptr);
    }

private:
    Counter *ptr_counter;
};