#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

bool readFile(const std::string &path, std::string &data) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!ifs.is_open())
        return false;
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);
    data.assign(bytes.data(), fileSize);
    return true;
}

int main() {
    std::string  out;
    std:: cout << readFile("/Users/linchengzhong/Desktop/query.lua", out) << std::endl;
    std::cout << out << std::endl;
}