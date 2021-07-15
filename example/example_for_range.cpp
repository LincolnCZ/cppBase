#include <iostream>
#include <vector>
#include <map>

void test_vector() {
    std::cout << "====================test_vector====================" << std::endl;
    std::vector<int> v = {0, 1, 2, 3, 4, 5};

    std::cout << "range for------------" << std::endl;

    for (const int &i : v) // access by const reference
        std::cout << i << ' ';
    std::cout << '\n';

    for (auto i : v) // access by value, the type of i is int
        std::cout << i << ' ';
    std::cout << '\n';

    for (auto &&i : v) // access by forwarding reference, the type of i is int&
        std::cout << i << ' ';
    std::cout << '\n';

    const auto &cv = v;

    for (auto &&i : cv) // access by f-d reference, the type of i is const int&
        std::cout << i << ' ';
    std::cout << '\n';

    for (int n : {0, 1, 2, 3, 4, 5}) // the initializer may be a braced-init-list
        std::cout << n << ' ';
    std::cout << '\n';

    int a[] = {0, 1, 2, 3, 4, 5};
    for (int n : a) // the initializer may be an array
        std::cout << n << ' ';
    std::cout << '\n';

    for ([[maybe_unused]] int n : a)
        std::cout << 1 << ' '; // the loop variable need not be used
    std::cout << '\n';

    std::cout << "change value" << std::endl;
    for (auto &i:v) {
        if (i == 2) {
            i = 20;
        }
    }

    for (const auto &i: v) {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::cout << "erase value" << std::endl;
    for (auto it = v.begin(); it != v.end();) {
        if (*it % 2 == 1) {
            it = v.erase(it);
            continue;
        }
        it++;
    }

    for (const auto &i: v) {
        std::cout << i << " ";
    }
    std::cout << "\n";
}

void test_map() {
    std::cout << "====================test_map====================" << std::endl;
    std::map<int, std::string> m = {{1, "one"},
                                    {2, "two"},
                                    {3, "three"},
                                    {4, "four"},
                                    {5, "five"}};

    std::cout << "range for------------" << std::endl;
    for (const auto &i :m)
        std::cout << i.first << " " << i.second << std::endl;

    std::cout << "change value--------------------" << std::endl;
    for (auto &i:m) {
        if (i.first % 2 == 1) {
            i.second += std::string("plus");
        }
    }

    for (const auto &i :m)
        std::cout << i.first << " " << i.second << std::endl;

    std::cout << "erase value--------------------" << std::endl;
    for (auto it = m.begin(); it != m.end();) {
        if (it->first % 2 == 1) {
            it = m.erase(it);
            continue;
        }
        ++it;
    }

    for (const auto &i :m)
        std::cout << i.first << " " << i.second << std::endl;
}

int main() {
    test_vector();
    test_map();
}