// t3b: one producer, one consumer, a condition_variable.
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <thread>
int main()
{
    std::mutex m; std::condition_variable cv; std::queue<int> q; bool done = false; long sum = 0;
    std::thread c([&] { std::unique_lock<std::mutex> lk(m); for (;;) { cv.wait(lk, [&] { return !q.empty() || done; }); while (!q.empty()) { sum += q.front(); q.pop(); } if (done) break; } });
    std::thread p([&] { for (int i = 1; i <= 500; ++i) { { std::lock_guard<std::mutex> g(m); q.push(i); } cv.notify_one(); } });
    p.join(); { std::lock_guard<std::mutex> g(m); done = true; } cv.notify_one(); c.join();
    std::puts(sum == 500L * 501 / 2 ? "PASS condition_variable" : "FAIL condition_variable");
    std::puts(sum == 500L * 501 / 2 ? "t3b OK" : "t3b FAILED");
    return sum == 500L * 501 / 2 ? 0 : 20;
}
