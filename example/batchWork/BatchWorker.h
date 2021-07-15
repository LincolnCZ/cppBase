#pragma once

#include <mutex>
#include <queue>
#include <future>
#include <memory>

struct BatchRequest {
    std::vector<int> input;
    std::promise<std::vector<int>> result;
};

class BatchWorker {
public:
    BatchWorker(int batch, int wait_ms);

    BatchWorker(const BatchWorker &) = delete;

    void operator=(const BatchWorker &) = delete;

    void start(int thread_num);

    std::future<std::vector<int>> add(const std::vector<int> &img);

private:
    static void mainloop(BatchWorker *worker);

    int work();

    int batch_;
    int wait_ms_;
    std::vector<std::thread> threads_;

    std::mutex mutex_;
    std::queue<std::shared_ptr<BatchRequest>> queue_;
    int64_t last_time_{};
};