#include <iostream>
#include <sstream>
#include <openssl/md5.h>
#include <set>

using namespace std;

static std::string dumpHexString(void *BinBuffer, unsigned int BufferLen) {
    static char HexTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    char *pSrc = (char *) BinBuffer;
    std::ostringstream oss;

    for (unsigned int i = 0; i < BufferLen; i++) {
        oss << HexTable[pSrc[i] >> 4 & 0x0f]
            << HexTable[pSrc[i] & 0x0f];
    }
    return oss.str();
}

static string md5_string(const string &content) {
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((const unsigned char *) content.c_str(), content.size(), result);
    return dumpHexString(result, MD5_DIGEST_LENGTH);
}


static uint64_t str2ull(std::string strInput) {
    uint64_t res = 0ull;
    const char *start = strInput.c_str();
    const char *end = start + strInput.size();
    for (const char *p = start; p < end; p++) {
        if (*p >= '0' && *p <= '9') {
            res = res * 10 + (*p - '0');
        } else {
            break;
        }
    }
    return res;
}

int main() {
    string t1("a1bcd3");
    cout << str2ull(t1) << endl;
    string t2("adfadf");
    cout << str2ull(t2) << endl;
}
