#include <iostream>
#include <utility>
#include <cstring>
#include <memory>
#include <map>
#include <functional>

using namespace std;
using namespace std::placeholders; // _1, _2

template<typename T, class... Args>
// 可变参数模板
std::unique_ptr<T> make_unique(Args &&... args) { // 可变参数模板的入口参数
    return std::unique_ptr<T>(
            new T(std::forward<Args>(args)...));// 完美转发
}

class Foo {
public:
    void printFoo(string &s) {
        cout << "printFoo():" << s << num << endl;
    }

private:
    int num = 1;
};

void print(string &s1, int a) {
    cout << "print():" << s1 << a << endl;
}

void (*printPtr)(string s);

int main() {
    std::function<void(string &)> f1 = std::bind(print, _1, 2);
    string s = "hello";
    f1(s);

    Foo foo;
    std::function<void(string &s)> f2 = std::bind(&Foo::printFoo, &foo, _1);
    f2(s);
}