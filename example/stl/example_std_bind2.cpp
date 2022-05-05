#include <string>
#include <functional>
#include <memory>
#include <iostream>

using namespace std::placeholders;

class Test {
private:
    std::string data;
};

typedef std::shared_ptr<Test> TestPtr;
typedef std::function<void(const TestPtr &ptr, const std::string &str)> FuncRef;
typedef std::function<void()> FuncVal;

class Foo {
public:
    void print(const TestPtr &ptr, const std::string &str) {
        std::cout << "ptr count:" << ptr.use_count() << " str:" << str << std::endl;
    }
};

int main(int argc, char **argv) {
    Foo f;
    TestPtr ptr(new Test);
    std::cout << "main ptr count:" << ptr.use_count() << std::endl;

    /// 不预绑定的参数要用std::placeholders(占位符)的形式占位，从_1开始，依次递增，是以引用传递的形式；
    FuncRef funcRef;
    {
        funcRef = std::bind(&Foo::print, &f, _1, _2);
    }
    funcRef(ptr, "hello");

    /// 预绑定的参数是以值传递的形式
    FuncVal funcVal;
    {
        std::string str("world");
        funcVal = std::bind(&Foo::print, &f, ptr, str); // 预绑定参数以值传递方式，所以ptr引用增加1, str 也会进行拷贝
    }
    funcVal();
}