#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>

using namespace std;

void work(condition_variable &cv, int &result) {
    // 假装我们计算了很久
    this_thread::sleep_for(std::chrono::seconds(2));
    result = 42;
    cv.notify_one();
}

int main() {
    condition_variable cv;
    mutex cv_mut;
    int result;

    std::thread th{work, ref(cv), ref(result)};
    th.detach();
    // 干一些其他事
    cout << "I am waiting now" << endl;
    std::unique_lock<mutex> lock{cv_mut};
    cv.wait(lock);
    cout << "Answer: " << result << endl;
}