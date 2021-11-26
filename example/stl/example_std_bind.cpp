#include <random>
#include <iostream>
#include <memory>
#include <functional>

void f(int n1, int n2, int n3, const int &n4, int n5) {
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

int g(int n1) {
    return n1;
}

struct Foo {
    void print_sum(int n1, int n2) {
        std::cout << n1 + n2 << '\n';
    }

    int data = 10;
};

int main() {
    using namespace std::placeholders;  // for _1, _2, _3...

    std::cout << "1) argument reordering and pass-by-reference: ";
    int n = 7;
    // (_1 and _2 are from std::placeholders, and represent future
    // arguments that will be passed to f1)
    auto f1 = std::bind(f, _2, 42, _1, std::cref(n), n);
    n = 10;
    f1(1, 2, 1001); // 1 is bound by _1, 2 is bound by _2, 1001 is unused  // 输出：2 42 1 10 7
    // makes a call to f(2, 42, 1, n, 7)

    std::cout << "2) nested bind subexpressions share the placeholders: ";
    auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5);
    f2(10, 11, 12); // makes a call to f(12, g(12), 12, 4, 5);   // 输出：12 12 12 4 5

    std::cout << "3) bind a RNG with a distribution: ";
    std::default_random_engine e;
    std::uniform_int_distribution<> d(0, 10);
    auto rnd = std::bind(d, e); // a copy of e is stored in rnd
    for (int n = 0; n < 10; ++n)
        std::cout << rnd() << ' '; // 输出：1 5 0 2 0 8 2 2 10 8
    std::cout << '\n';

    std::cout << "4) bind to a pointer to member function: ";
    Foo foo;
    // bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址，这是因为对象的成员函数需要有this指针。
    auto f4 = std::bind(&Foo::print_sum, &foo, 95, _1);
    f4(5); // 输出：100

    std::cout << "5) bind to a pointer to data member: ";
    auto f5 = std::bind(&Foo::data, _1);
    std::cout << f5(foo) << '\n'; // 输出：10

    std::cout << "6) use smart pointers to call members of the referenced objects: ";
    std::cout << f5(std::make_shared<Foo>(foo)) << '\n';// 输出：10
}