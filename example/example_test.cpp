#include <algorithm>
#include <vector>
#include <stdio.h>
#include <iostream>

const static int kRollPerSeconds_ = 60 * 60 * 24;

int main() {
    time_t now = ::time(NULL);
    std::cout << now << std::endl;
    time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
    std::cout << thisPeriod_ << std::endl;
}