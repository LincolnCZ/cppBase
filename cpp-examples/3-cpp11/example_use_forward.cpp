#include <utility>  // std::forward
#include <cstdio>  // puts
#include <iostream>
#include <memory>

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

namespace version4 {
    /// 刨除不常见的 const 右值的情况，我们来具体分析一下常见的三种场景（先限定单参数的情况）：
    ///   当给定的参数是 const 左值（如 const Obj&）时，Args 被推导为 const Obj&，这样，在引用坍缩后，Args&& 仍然是 const Obj&。
    ///   当给定的参数是非 const 左值（如 Obj&）时，Args 被推导为 Obj&，这样，在引用坍缩后，Args&& 仍然是 Obj&。
    ///   当给定的参数是右值（如 Obj&&）时，Args 被推导为 Obj，这样，Args&& 当然仍保持为 Obj&&。
    /// 回顾一下，我们这里要使用 forward 的原因是，所有的变量都是左值，因此，如果我们要保持“右值性”，就得使用强制类型转换。forward 所做的
    ///   事情，本质上就是 static_cast<Args&&>(args)，右值被转换成右值引用（xvalue），左值仍保持为左值引用（由于引用坍缩）。
    template<typename T, class... Args> // 可变参数模板
    std::unique_ptr<T> make_unique(Args &&... args) { // 可变参数模板的入口参数
        return std::unique_ptr<T>(
                new T(std::forward<Args>(args)...));// 完美转发
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