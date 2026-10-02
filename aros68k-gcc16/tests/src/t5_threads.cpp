// t5: threads. pthreads, then std::thread, mutex, condition variable, atomics.
#include <cstdio>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

static int pass, fail;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; std::printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

static pthread_mutex_t pm = PTHREAD_MUTEX_INITIALIZER;
static long pcount;
static void *pworker(void *) { for (int i = 0; i < 5000; i++) { pthread_mutex_lock(&pm); pcount++; pthread_mutex_unlock(&pm); } return nullptr; }

int main()
{
    pthread_t t[4];
    for (auto &x : t) pthread_create(&x, nullptr, pworker, nullptr);
    for (auto &x : t) pthread_join(x, nullptr);
    CHECK("pthread + mutex", pcount == 20000);

    std::mutex m; long count = 0; std::atomic<int> at{0};
    std::vector<std::thread> ts;
    for (int i = 0; i < 4; i++) ts.emplace_back([&] { for (int j = 0; j < 5000; j++) { { std::lock_guard<std::mutex> l(m); count++; } at.fetch_add(1); } });
    for (auto &x : ts) x.join();
    CHECK("std::thread + mutex", count == 20000);
    CHECK("atomic", at.load() == 20000);

    std::condition_variable cv; bool ready = false; int pings = 0;
    std::thread pong([&] { for (int i = 0; i < 100; i++) { std::unique_lock<std::mutex> l(m); cv.wait(l, [&] { return ready; }); ready = false; pings++; cv.notify_one(); } });
    for (int i = 0; i < 100; i++) { std::unique_lock<std::mutex> l(m); ready = true; cv.notify_one(); cv.wait(l, [&] { return !ready; }); }
    pong.join();
    CHECK("condition variable ping-pong", pings == 100);
    std::printf("t5 threads: %d passed, %d failed\n", pass, fail);
    std::printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
