#include <utility>  // std::forward
#include <cstdio>  // puts
#include <iostream>

class shape {
public:
    virtual ~shape() = default;
};

class circle : public shape {
public:
    circle() = default;

    ~circle() override = default;
};

void foo(const shape &) {
    puts("foo(const shape&)");
}

void foo(shape &&) {
    puts("foo(shape&&)");
}

namespace version1 {
    void bar(const shape &s) {
        puts("bar(const shape&)");
        foo(s);
    }

    void bar(shape &&s) {
        puts("bar(shape&&)");
        foo(s); /// 存在问题：shape &&s 是一个左值，会调用 foo(const shape&)
    }
}

namespace version2 {
    void bar(const shape &s) {
        puts("bar(const shape&)");
        foo(s);
    }

    void bar(shape &&s) {
        puts("bar(shape&&)");
        foo(std::move(s)); /// 使用 std::move(s) 将s转成右值，调用foo(shape&&)
    }
}

namespace version3 {
    /// 对于 template <typename T> bar(T&&) 这样的代码（即“万能引用”（universal reference）），
    /// 如果传递过去的参数是左值，T 的推导结果是左值引用；如果传递过去的参数是右值，T 的推导结果是参数的类型本身。
    ///   如果 T 是左值引用，那 T&& 的结果仍然是左值引用——即 type& && 坍缩成了 type&。
    ///   如果 T 是一个实际类型，那 T&& 的结果自然就是一个右值引用。
    template<typename T>
    void bar(T &&s) {
        foo(std::forward<T>(s)); /// 使用完美转发来调用对应的构造函数
    }
}

int main() {
    std::cout << "---version1---" << std::endl;
    version1::bar(circle());

    std::cout << "---version2---" << std::endl;
    version2::bar(circle());

    std::cout << "---version3---" << std::endl;
    circle temp;
    version3::bar(temp);
    version3::bar(circle());
    version3::bar(std::move(temp));
}