// SPDX-License-Identifier: MIT
// t17: std::condition_variable_any with std::mutex (4 producers, 1 consumer),
// with std::shared_mutex (notify_all to 8 waiters), with a lock of its own
// built on atomic_flag, and wait_for timing out.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>
struct FlagLock {
    std::atomic_flag f;
    void lock() { while (f.test_and_set(std::memory_order_acquire)) std::this_thread::yield(); }
    void unlock() { f.clear(std::memory_order_release); }
};
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    std::mutex m; std::condition_variable_any cv; std::queue<int> q; bool done = false; long sum = 0;
    std::thread consumer([&] { std::unique_lock<std::mutex> lk(m); for (;;) { cv.wait(lk, [&] { return !q.empty() || done; }); while (!q.empty()) { sum += q.front(); q.pop(); } if (done) break; } });
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t) ts.emplace_back([&, t] { for (int i = 1; i <= 200; ++i) { { std::lock_guard<std::mutex> g(m); q.push(i + t * 1000); } cv.notify_one(); } });
    for (auto &t : ts) t.join(); ts.clear();
    { std::lock_guard<std::mutex> g(m); done = true; } cv.notify_one(); consumer.join();
    check(sum == 4 * (200 * 201 / 2) + 200 * 6000L, "with std::mutex: 4 producers, 1 consumer");
    std::shared_mutex sm; std::condition_variable_any cv2; bool go = false; std::atomic<int> woke{0}, waiting{0};
    for (int i = 0; i < 8; ++i) ts.emplace_back([&] { std::unique_lock<std::shared_mutex> lk(sm); ++waiting; cv2.wait(lk, [&] { return go; }); ++woke; });
    while (waiting < 8) std::this_thread::yield();
    { std::unique_lock<std::shared_mutex> lk(sm); go = true; } cv2.notify_all();
    for (auto &t : ts) t.join(); ts.clear();
    check(woke == 8, "with std::shared_mutex: notify_all wakes 8");
    FlagLock fl; std::condition_variable_any cv3; int turn = 0; const int R = 200;
    std::thread other([&] { for (int i = 0; i < R; ++i) { std::unique_lock<FlagLock> lk(fl); cv3.wait(lk, [&] { return turn % 2 == 1; }); ++turn; cv3.notify_one(); } });
    for (int i = 0; i < R; ++i) { std::unique_lock<FlagLock> lk(fl); cv3.wait(lk, [&] { return turn % 2 == 0; }); ++turn; cv3.notify_one(); }
    other.join();
    check(turn == 2 * R, "with a lock of its own: 200 rounds of turn-taking");
    std::unique_lock<std::mutex> lk(m);
    auto t0 = std::chrono::steady_clock::now();
    bool r = cv.wait_for(lk, std::chrono::milliseconds(100), [] { return false; });
    long ms = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    std::printf("wait_for returned after %ld ms\n", ms);
    check(!r && ms >= 80 && ms < 5000, "wait_for times out after about 100 ms");
    std::puts(fails ? "t17 FAILED" : "t17 OK");
    return fails ? 20 : 0;
}
