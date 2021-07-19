#include "smartpoint.h"

using namespace std;

int main() {
    SmartPointer p(new Object());

    p.GetObject().a = 10;
    p.GetObject().b = 20;

    cout << "a_val = " << p->a << "\tb_val = " << p->b << endl;

    SmartPointer p2(p);
    cout << "a = " << p2->a << "\tb = " << p2->b << endl;

    return 0;
}