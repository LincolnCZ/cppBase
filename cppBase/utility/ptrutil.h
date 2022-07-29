#pragma once

#include <memory>
#include <functional>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

template<typename Function>
class DeferAction {
    Function _func;

public:
    DeferAction(Function &f) : _func(f) {}
    ~DeferAction() { _func(); }
};

// defer 的调用顺序满足FILO
template<typename Function>
DeferAction<Function> defer(Function f) {
    return DeferAction<Function>(f);
}

// make_unique 生成默认构造的 unique_ptr
template<typename T>
std::unique_ptr<T> make_unique() {
    return std::unique_ptr<T>(new T);
}

// make_unique 生成指定对象的 unique_ptr
template<typename T>
std::unique_ptr<T> make_unique(T *p) {
    return std::unique_ptr<T>(p);
}

// make_unique 生成自定义删除函数的 unique_ptr
template<typename T>
std::unique_ptr<T, void (*)(T *)> make_unique(T *p, void(*d)(T *)) {
    return std::unique_ptr<T, void (*)(T *)>(p, d);
}

// make_unique 生成自定义删除函数的 unique_ptr
template<typename T>
std::unique_ptr<T, std::function<void(T *)>> make_unique(T *p, std::function<void(T *)> d) {
    return std::unique_ptr<T, std::function<void(T *)>>(p, d);
}

template<typename T, class... Args> // 可变参数模板
std::unique_ptr<T> make_unique(Args &&... args) { // 可变参数模板的入口参数
    return std::unique_ptr<T>(
            new T(std::forward<Args>(args)...));// 完美转发
}

UTILITY_NAMESPACE_END