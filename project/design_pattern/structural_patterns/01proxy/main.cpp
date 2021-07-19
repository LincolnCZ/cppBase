// main.cpp
#include "proxy.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p){delete(p); (p)=NULL;} }
#endif

int main() {
    auto *proxy = new Proxy();
    proxy->Do(10); // Sorry, too little money
    proxy->Do(100); // Recharge 100

    SAFE_DELETE(proxy);

    return 0;
}