#include <iostream>
#include <vector>
#include <map>
#include <set>

/// auto 实际使用的规则类似于函数模板参数的推导规则。当你写了一个含 auto 的表达式时，相当于把 auto 替换为模板参数的结果。举具体的例子：
///   auto a = expr; 意味着用 expr 去匹配一个假想的 template <typename T> f(T) 函数模板，结果为值类型。
///   const auto& a = expr; 意味着用 expr 去匹配一个假想的 template <typename T> f(const T&) 函数模板，结果为常左值引用类型。
///   auto&& a = expr; 意味着用 expr 去匹配一个假想的 template <typename T> f(T&&) 函数模板，根据转发引用和引用坍缩规则，结果是
///      一个跟 expr 值类别相同的引用类型。

template<typename T>
std::ostream &operator<<(std::ostream &oStr, const std::vector<T> &vec) {
    for (const auto &i: vec) {
        oStr << i << ' ';
    }
    return oStr;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &oStr, const std::map<K, V> &m) {
    for (const auto &i: m) {
        oStr << i.first << " " << i.second << "\n";
    }
    return oStr;
}

template<typename T>
std::ostream &operator<<(std::ostream &oStr, const std::set<T> &s) {
    for (const auto &i: s) {
        oStr << i << ' ';
    }
    return oStr;
}

void test_vector() {
    std::cout << "====================test_vector====================" << std::endl;
    std::vector<int> v = {0, 1, 2, 3, 4, 5};

    std::cout << "---range for---" << std::endl;
    for (const auto &i: v) // access by const reference
        std::cout << i << ' ';
    std::cout << '\n';

    /// 转发引用的常见用法 auto&&。
    /// 这么写着还真方便啊，也不用管参数是不是 const，及到底是左值还是右值了。
    for (auto &&i: v) /// access by forwarding reference, the type of i is int&
        std::cout << i << ' ';
    std::cout << '\n';
    for (auto &&i: {0, 1, 2, 3, 4, 5}) { // the type of is const int&
        std::cout << i << ' ';
    }
    std::cout << '\n';

    for (const auto &i: {0, 1, 2, 3, 4, 5}) // the initializer may be a braced-init-list
        std::cout << i << ' ';
    std::cout << '\n';

    std::cout << "---change value---" << std::endl;
    for (auto &i: v) {
        if (i % 2 == 0) {
            i *= 10;
        }
    }
    std::cout << v << std::endl;

    std::cout << "---erase value---" << std::endl;
    for (auto it = v.begin(); it != v.end();) {
        if (*it % 2 == 1) {
            it = v.erase(it);
            continue;
        }
        it++;
    }
    std::cout << v << std::endl;
}

void test_map() {
    std::cout << "====================test_map====================" << std::endl;
    std::map<int, std::string> m = {{1, "one"},
                                    {2, "two"},
                                    {3, "three"},
                                    {4, "four"},
                                    {5, "five"}};

    std::cout << "---range for---" << std::endl;
    for (const auto &i: m)
        std::cout << i.first << " " << i.second << std::endl;

    std::cout << "---change value---" << std::endl;
    for (auto &i: m) {
        if (i.first % 2 == 1) {
            i.second += std::string("plus");
        }
    }
    std::cout << m << std::endl;

    std::cout << "---erase value---" << std::endl;
    for (auto it = m.begin(); it != m.end();) {
        if (it->first % 2 == 1) {
            it = m.erase(it);
            // m.erase(it++); // 这种写法也可以
        } else {
            ++it;
        }
    }
    std::cout << m << std::endl;
}

void test_set() {
    std::cout << "====================test_set====================" << std::endl;
    std::set<std::string> s = {"one", "two", "three", "four", "five"};
    std::cout << "---range for---" << std::endl;
    for (const auto &i: s) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "---erase value---" << std::endl;
    for (auto it = s.begin(); it != s.end();) { /// 注意：set 返回的迭代器是const 类型的
        if (*it == "three") {
            it = s.erase(it);
            continue;
        }
        ++it;
    }
    std::cout << s << std::endl;
}

int main() {
    test_vector();
    test_map();
    test_set();
}