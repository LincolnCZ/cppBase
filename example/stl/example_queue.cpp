#include <queue>
#include <iostream>

int main() {
    std::queue<int> q1;
    for (int i = 0; i < 5; ++i) {
        //------------------Modifiers------------------
        // q1.push(i);
        q1.emplace(i);
    }

    //------------------Capacity------------------
    std::cout << "q1.size():" << q1.size() << '\t q1.empty():' << q1.empty() << std::endl;

    std::queue<int> q2(q1);
    std::cout << q2.size() << '\n';

    //------------------Element access------------------
    //q.front():access the first element
    std::cout << "q1.front():" << q1.front() << std::endl;

    //q.back():access the last element
    std::cout << "q1.back():" << q1.back() << std::endl;

    //------------------Modifiers------------------
    //q.pop():removes the first element
    q1.pop();
    std::cout << "after q1.pop(), size:" << q1.size() << std::endl;

    //q.swap()
    q1.swap(q2);
    std::cout << "after q1.swap(q2), q1 size: " << q1.size() << "\tq2 size:" << q2.size() << std::endl;
}
