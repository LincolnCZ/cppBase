#include "BatchWorker.h"
#include <iostream>

std::unique_ptr<BatchWorker> g_batchWorker;
std::mutex g_mutex;

void do_work(int id) {
    std::vector<int> input = {id, id + 1, id + 2};
    std::future<std::vector<int>> fut = g_batchWorker->add(input);
    std::vector<int> result = fut.get();

    std::lock_guard<std::mutex> lockGuard{g_mutex};

    std::cout << "id : " << id << std::endl;
    std::cout << "\tinput :";
    for (const auto i: input) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    std::cout << "\tresult :";
    for (auto i: result) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;
}

int main() {
    g_batchWorker = std::unique_ptr<BatchWorker>(new BatchWorker(3, 100));
    g_batchWorker->start(2);

    for (int i = 0; i < 4; ++i) {
        std::thread th(do_work, i);
        th.detach();
    }

    while (true) {}
}