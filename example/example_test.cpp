#include <iostream>

int main() {
    int uri = 2010 << 8 | 10;
    std::cout << uri << std::endl;

    int i = 514570;
    int r = i >> 8;
    std::cout << r << std::endl;
}