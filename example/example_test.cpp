#include <algorithm>
#include <vector>
#include <stdio.h>
#include <iostream>

const static int kRollPerSeconds_ = 60 * 60 * 24;

int main() {
    int uri = 1001 << 8 | 80;
    std::cout << uri << std::endl;
}