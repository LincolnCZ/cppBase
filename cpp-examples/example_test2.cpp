#include <iostream>
#include <utility>
#include <cstring>
#include <memory>

using namespace std;

template<typename T, class... Args>
// 可变参数模板
std::unique_ptr<T> make_unique(Args &&... args) { // 可变参数模板的入口参数
    return std::unique_ptr<T>(
            new T(std::forward<Args>(args)...));// 完美转发
}

int main() {
    std::unique_ptr<int> p = make_unique<int>(42);
    std::cout << *p << endl;
}