#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "BatchWorker.h"

std::vector<std::vector<int>> fake_result(const std::vector<std::vector<int>> &input) {
    std::vector<std::vector<int>> result;
    for (const auto &it : input) {
        std::vector<int> tmp;
        tmp.reserve(it.size());

        for (int sec : it) {
            tmp.push_back(sec * 2);
        }
        result.push_back(tmp);
    }
    return result;
}

static int64_t getTimeNow() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

BatchWorker::BatchWorker(int batch, int wait_ms)
        : batch_(batch), wait_ms_(wait_ms) {
}

void BatchWorker::start(int thread_num) {
    for (int i = 0; i < thread_num; i++) {
        std::thread th(BatchWorker::mainloop, this);
        th.detach();
        threads_.push_back(std::move(th));
    }
}

std::future<std::vector<int>> BatchWorker::add(const std::vector<int> &img) {
    std::shared_ptr<BatchRequest> req(new BatchRequest);
    req->input = img;
    std::future<std::vector<int>> fut = req->result.get_future();

    std::lock_guard<std::mutex> lockGuard{mutex_};
    queue_.push(req);
    return fut;
}

void BatchWorker::mainloop(BatchWorker *worker) {
    while (1) {
        int c = worker->work();
        if (c == 0) {
            usleep(worker->wait_ms_ * 1000);
        }
    }
}

int BatchWorker::work() {
    std::vector<std::shared_ptr<BatchRequest>> reqs;
    std::vector<std::vector<int>> imgs;
    int64_t now = getTimeNow();

    mutex_.lock();
    if (queue_.size() < batch_ && now < last_time_ + wait_ms_) {
        mutex_.unlock();
        return 0;
    }
    last_time_ = now;

    for (int i = 0; i < batch_ && !queue_.empty(); i++) {
        std::shared_ptr<BatchRequest> r = queue_.front();
        imgs.push_back(r->input);
        reqs.push_back(r);
        queue_.pop();
    }
    mutex_.unlock();

    if (reqs.empty()) {
        return 0;
    }

    // call classifier
    std::vector<std::vector<int>> scorelist = fake_result(imgs);
    for (int i = 0; i < reqs.size(); i++) {
        reqs[i]->result.set_value(scorelist[i]);
    }
    return reqs.size();
}