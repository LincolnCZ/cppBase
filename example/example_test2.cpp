#include <mutex>
#include <iostream>

int main() {
    const int max_name = 80;
    char name[max_name];

    char fmt[10];
    sprintf(fmt, "%%%ds", max_name - 1);
    std::cout << "fmt:" << fmt << std::endl;
    scanf(fmt, name);
    printf("%s\n", name);
    std::cout << "name:" << name << std::endl;
}