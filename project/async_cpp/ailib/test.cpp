#include <cstdio>
#include <string>
#include <vector>
#include <sys/time.h>
#include <unistd.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "cover_qc_models_api.h"


inline long getCurrentMicroTime2() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
}

void infer_task(yyai::CoverQualityModelsApi& cover_qc_model, std::string& image_file, int thread_id) {
    bool loop = true;
    int count = 1;
    do {
        long t1 = getCurrentMicroTime2();
        std::string predict_results;
        int ret = cover_qc_model.inference("12345", image_file, predict_results);
        long t2 = getCurrentMicroTime2();
        printf("tid %d : time %ld us, code %d: %s\n", thread_id, (t2 - t1), ret, predict_results.c_str());
        count --;
    } while (loop && count > 0);
}

int main(int argc, char** argv) {
    std::string model_dir(argv[1]);
    std::string image_file(argv[2]);
    int thread_count = atoi(argv[3]);

    yyai::conf_param_t params;
    params.gpu_id = 0;
    params.model_pool_size = 1;
    params.models_dir = model_dir;

    yyai::CoverQualityModelsApi cover_qc_model(params);

    for (int i = 0; i < thread_count; i++) {
        std::thread th(infer_task, std::ref(cover_qc_model), std::ref(image_file), i);
        // threads.push_back(&th);
        th.detach();
    }
    getchar();

    return 0;
}
