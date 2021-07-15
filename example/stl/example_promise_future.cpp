#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>

void accumulate(std::vector<int>::iterator first,
                std::vector<int>::iterator last,
                std::promise<int> accumulate_promise) {
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum);  // Notify future
}

void do_work(std::promise<void> barrier) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    barrier.set_value();
}

int main() {
    // Demonstrate using promise<int> to transmit a result between threads.
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    std::promise<int> accumulate_promise;
    std::future<int> accumulate_future = accumulate_promise.get_future();
    std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
                            std::move(accumulate_promise)); // 直接使用std::move，这样主线程就可以不用管理accumulate_promise

    // future::get() will wait until the future has a valid result and retrieves it.
    // Calling wait() before get() is not needed
    //accumulate_future.wait();  // wait for result
    std::cout << "result=" << accumulate_future.get() << '\n';
    work_thread.join();  // wait for thread completion

    // Demonstrate using promise<void> to signal state between threads.
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread new_work_thread(do_work, std::move(barrier));
    barrier_future.wait();
    new_work_thread.join();
    std::cout << "wait finish" << std::endl;
}

// 这里有两个要点，从代码里看不出来，特别说明一下：
// 1. 一个 future 上只能调用一次 get 函数，第二次调用为未定义行为，通常导致程序崩溃。
// 2. 这样一来，自然一个 future 是不能直接在多个线程里用的。
//      第 2 点则是可以解决的。要么直接拿 future 来移动构造一个 shared_future，要么调用 future 的 share 方法来生成一个 shared_future，
//      结果就可以在多个线程里用了——当然，每个 shared_future 上仍然还是只能调用一次 get 函数。
//