// SPDX-License-Identifier: MIT
// t15: std::jthread and std::stop_token: the destructor asks the thread to stop
// and joins it, stop_callback runs (also when registered after the stop),
// condition_variable_any::wait with a stop_token wakes on request_stop, and a
// vector of 12 waiting jthreads is stopped and cleared.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    std::atomic<long> spins{0}; std::atomic<bool> stopped{false};
    {
        std::jthread j([&](std::stop_token st) { while (!st.stop_requested()) { ++spins; std::this_thread::yield(); } stopped = true; });
        while (spins < 100) std::this_thread::yield();
    }
    check(stopped, "jthread destructor: request_stop and join");
    std::stop_source src; std::atomic<int> cb{0};
    {
        std::stop_callback c(src.get_token(), [&] { ++cb; });
        std::jthread j([&] { src.request_stop(); });
    }
    std::stop_callback late(src.get_token(), [&] { ++cb; });
    check(cb == 2, "stop_callback: from another thread, and registered after the stop");
    std::mutex m; std::condition_variable_any cv; std::atomic<bool> woke{false}; bool res = true;
    std::jthread w([&](std::stop_token st) { std::unique_lock<std::mutex> lk(m); res = cv.wait(lk, st, [] { return false; }); woke = true; });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    w.request_stop(); w.join();
    check(woke && !res, "condition_variable_any::wait(stop_token) wakes on request_stop");
    std::atomic<int> ends{0}, started{0};
    {
        std::vector<std::jthread> v;
        for (int i = 0; i < 12; ++i) v.emplace_back([&](std::stop_token st) {
            std::mutex mm; std::condition_variable_any c; std::unique_lock<std::mutex> l(mm);
            ++started; c.wait(l, st, [] { return false; }); ++ends;
        });
        while (started < 12) std::this_thread::yield();
    }
    check(ends == 12, "12 waiting jthreads stopped by their destructors");
    std::puts(fails ? "t15 FAILED" : "t15 OK");
    return fails ? 20 : 0;
}
