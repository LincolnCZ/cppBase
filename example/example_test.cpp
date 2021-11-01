#include <iostream>

void parse(const std::string &s, const std::string &delim, std::string &liveId, std::string &version) {
    size_t pos = 0;
    size_t index = s.find(delim);
    if (index != std::string::npos) {
        liveId = s.substr(0, index);
        pos = index + delim.size();

        version = s.substr(pos);
    }
}

int main() {
    int code = 2010 << 8 | 10;
    std::cout << code << std::endl;
}