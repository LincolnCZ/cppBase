#include <sstream>
#include <string>
#include <stdio.h>
#include <sys/time.h>

std::string use_snprintf(int a) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", a);
    return buf;
}

std::string use_stringstream(int a) {
    std::ostringstream oss;
    oss << a;
    return oss.str();
}

const int LOOPS = 1000000;

void *thread(void *p) {
    std::string (*foo)(int) = (std::string (*)(int)) p;
    for (int i = 0; i < LOOPS; ++i)
        foo(i + 1);
    return p;
}

double run_with_threads(int threads, std::string (*foo)(int)) {
    timeval start, end;
    gettimeofday(&start, nullptr);

    pthread_t *tids = new pthread_t[threads];
    for (int i = 0; i < threads; ++i)
        pthread_create(&tids[i], nullptr, thread, (void *) foo);
    for (int i = 0; i < threads; ++i)
        pthread_join(tids[i], nullptr);
    delete[] tids;

    gettimeofday(&end, nullptr);

    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
}

void test_with_threads(int threads) {
    printf("%d threads:\n", threads);
    double time_snprintf = run_with_threads(threads, use_snprintf);
    double time_stringstream = run_with_threads(threads, use_stringstream);
    printf("snprintf:        %f\n", time_snprintf);
    printf("stringstream:    %f\n", time_stringstream);
    printf("stream/snprintf: %f\n", time_stringstream / time_snprintf);
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc > 1) {
        test_with_threads(atoi(argv[1]));
    } else {
        test_with_threads(1);
        test_with_threads(2);
        test_with_threads(4);
        test_with_threads(10);
        test_with_threads(20);
    }
}