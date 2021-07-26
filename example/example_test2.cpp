#include <iostream>
#include <memory>

using namespace std;

class B;

class A {
public:
    shared_ptr<B> pb;

    ~A() {
        cout << "kill A\n";
    }
};

class B {
public:
    weak_ptr<A> pa;

    ~B() {
        cout << "kill B\n";
    }
};

int main(int argc, char **argv) {

    shared_ptr<A> sa(new A());
    {
        shared_ptr<B> sb(new B());
        if (sa && sb) {
            sa->pb = sb;
            sb->pa = sa;
        }
        cout << "sb use count:" << sb.use_count() << endl;
    }
    cout << "sa use count:" << sa.use_count() << endl;
    cout << "sa.pb use count:" << sa->pb.use_count() << endl;
    return 0;
}