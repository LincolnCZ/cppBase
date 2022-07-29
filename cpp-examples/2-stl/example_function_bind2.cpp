#include <string>
#include <functional>
#include <memory>
#include <iostream>

using namespace std::placeholders;

/// 说明 bind 参数的值传递和引用传递
namespace test1 {
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

    void test() {
        Foo f;
        TestPtr ptr(new Test);
        std::cout << "main ptr count:" << ptr.use_count() << std::endl;

        /// 使用std::placeholders(占位符)的形式占位，从_1开始，依次递增，是以引用传递的形式；
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
}

/// 说明bind 拷贝的是实参类型(const char*)，不是形参类型(string)
namespace test2 {
    class Foo {
    public:
        void methodA() {};

        void methodInt(int a) {};

        void methodString(const std::string &str) {};
    };

    class Bar {
    public:
        void methodB() {};
    };

    void test() {
        std::function<void()> f1; // 无参数，无返回值

        Foo foo;
        f1 = std::bind(&Foo::methodA, &foo);
        f1(); // 调用 foo.methodA();

        Bar bar;
        f1 = std::bind(&Bar::methodB, &bar);
        f1(); // 调用 bar.methodB();

        f1 = std::bind(&Foo::methodInt, &foo, 42);
        f1(); // 调用 foo.methodInt(42);

        f1 = std::bind(&Foo::methodString, &foo, "hello");
        f1(); // 调用 foo.methodString("hello")
        /// 注意，bind 拷贝的是实参类型(const char*)，不是形参类型(string)
        /// 这里形参中的 string 对象的构造发生在调用 f1 的时候，而非 bind 的时候，
        /// 因此要留意bind的实参(cosnt char*)的生命期，它应该不短于f1的生命期。
        /// 必要时可通过 bind(&Foo::methodString, &foo, string(aTempBuf)) 来保证安全

        std::function<void(int)> f2; // int 参数，无返回值
        f2 = std::bind(&Foo::methodInt, &foo, _1);
        f2(53); // 调用 foo.methodInt(53);
    }
}

int main(int argc, char **argv) {
    test1::test();
    test2::test();
}