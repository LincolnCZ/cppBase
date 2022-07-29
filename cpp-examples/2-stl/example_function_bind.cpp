// 说明：https://blog.csdn.net/qq_38410730/article/details/103637778
// bind 原理：https://www.cnblogs.com/xusd-null/p/3698969.html

/// C++中有如下几种 可调用对象：函数、函数指针、lambda表达式、bind对象、函数对象。
///   函数对象：重载了函数调用运算符()的类的对象，即为函数对象
/// 由于可调用对象的定义方式比较多，但是函数的调用方式较为类似，因此需要使用一个统一的方式保存可调用对象或者传递可调用对象。
///   于是，std::function 就诞生了。

#include <functional>
#include <iostream>
#include <string>

using namespace std::placeholders;  // for _1, _2, _3...

struct Foo {
    explicit Foo(int num) : num_(num) {}

    void print_add(int i) const { std::cout << num_ + i << '\n'; }

    static void print_str(const std::string &str) {
        std::cout << "print : " << str << std::endl;
    }

    int num_;
};

// 普通函数
void print_num(int i) {
    std::cout << i << '\n';
}

void print_num2(int n1, int n2, int n3) {
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << std::endl;
}

// 函数指针
typedef void(*FunPtr)(int, int, int);

// 函数对象
struct PrintNum {
    void operator()(int i) const {
        std::cout << i << '\n';
    }
};

int main() {
    /// (1) store a free function
    std::function<void(int)> f_display = print_num;
    f_display(-9); // 输出：-9

    std::cout << "1) argument reordering and pass-by-reference: ";
    // (_1 and _2 are from std::placeholders, and represent future arguments that will be passed to f1)
    std::function<void(int, int)> f_display2 = std::bind(print_num2, _1, _2, 42);
    f_display2(1, 2); // 调用 print_nmu2(1, 2, 42);

    /// (2) store a lambda
    std::function<void()> f_display_42 = []() { print_num(42); };
    f_display_42(); // 输出：42

    /// (3) store a call to a member function
    std::function<void(const Foo &, int)> f_add_display = &Foo::print_add;
    const Foo foo(314159);
    f_add_display(foo, 1);    // 输出：314160

    /// (4) store a call to a member function and object
    using std::placeholders::_1;
    std::function<void(int)> f_add_display2 = std::bind(&Foo::print_add, foo, _1);
    f_add_display2(2); // 输出：314161

    /// (5) store a call to a member function and object ptr
    // bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址，这是因为对象的成员函数需要有this指针。
    std::function<void(int)> f_add_display3 = std::bind(&Foo::print_add, &foo, _1);
    f_add_display3(3); // 输出：314162

    /// (6) static member function
    std::function<void(const std::string &)> f_print_str = std::bind(&Foo::print_str, _1); // static 成员函数，不需要指定对象地址
    f_print_str("hello");

    /// (7) store a call to a function object 函数对象
    std::function<void(int)> f_display_obj = PrintNum();
    f_display_obj(18); // 输出：18
}