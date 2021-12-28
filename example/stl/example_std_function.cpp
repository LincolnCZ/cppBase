#include <functional>
#include <iostream>

// 说明：https://blog.csdn.net/qq_38410730/article/details/103637778
// bind 原理：https://www.cnblogs.com/xusd-null/p/3698969.html

struct Foo {
    Foo(int num) : num_(num) {}

    void print_add(int i) const { std::cout << num_ + i << '\n'; }

    int num_;
};

void print_num(int i) {
    std::cout << i << '\n';
}

struct PrintNum {
    void operator()(int i) const {
        std::cout << i << '\n';
    }
};

int main() {
    // (1) store a free function
    std::function<void(int)> f_display = print_num;
    f_display(-9); // 输出：-9

    /// (2) store a lambda
    std::function<void()> f_display_42 = []() { print_num(42); };
    f_display_42(); // 输出：42

    /// (3) store the result of a call to std::bind
    std::function<void()> f_display_31337 = std::bind(print_num, 31337);
    f_display_31337(); // 输出：31337

    // (4) store a call to a member function
    std::function<void(const Foo &, int)> f_add_display = &Foo::print_add;
    const Foo foo(314159);
    f_add_display(foo, 1);    // 输出：314160
    f_add_display(314159, 1); // 输出：314160

    // (5) store a call to a data member accessor
    std::function<int(Foo const &)> f_num = &Foo::num_;
    std::cout << "num_: " << f_num(foo) << '\n'; // 输出：num_: 314159

    // (6) store a call to a member function and object
    using std::placeholders::_1;
    std::function<void(int)> f_add_display2 = std::bind(&Foo::print_add, foo, _1);
    f_add_display2(2); // 输出：314161

    /// (7) store a call to a member function and object ptr
    std::function<void(int)> f_add_display3 = std::bind(&Foo::print_add, &foo, _1);
    f_add_display3(3); // 输出：314162

    // (8) store a call to a function object
    std::function<void(int)> f_display_obj = PrintNum();
    f_display_obj(18); // 输出：18
}