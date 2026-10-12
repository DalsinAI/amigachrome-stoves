// SPDX-License-Identifier: MIT
// t16: C++20 atomic wait/notify and what is built on it: a 500-round ping-pong
// between two threads, notify_all to 8 waiters, atomic_flag wait, std::latch,
// std::barrier (20 phases, 6 threads) and std::counting_semaphore (at most 3
// of 10 threads inside).
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdio>
#include <latch>
#include <semaphore>
#include <thread>
#include <vector>
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    const int N = 500; std::atomic<int> v{0};
    std::thread b([&] { for (int i = 0; i < N; ++i) { int x; while ((x = v.load()) != 2 * i + 1) v.wait(x); v.store(2 * i + 2); v.notify_one(); } });
    for (int i = 0; i < N; ++i) { int x; while ((x = v.load()) != 2 * i) v.wait(x); v.store(2 * i + 1); v.notify_one(); }
    b.join();
    check(v == 2 * N, "atomic wait/notify_one: 500-round ping-pong");
    std::atomic<bool> go{false}; std::atomic<int> woken{0}; std::vector<std::thread> ts;
    for (int i = 0; i < 8; ++i) ts.emplace_back([&] { go.wait(false); ++woken; });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    go = true; go.notify_all();
    for (auto &t : ts) t.join(); ts.clear();
    check(woken == 8, "atomic notify_all wakes 8 waiters");
    std::atomic_flag flag; std::atomic<bool> fseen{false};
    std::thread ft([&] { flag.wait(false); fseen = true; });
    flag.test_and_set(); flag.notify_one(); ft.join();
    check(fseen, "atomic_flag wait/notify");
    std::latch l(8); std::atomic<int> past{0};
    for (int i = 0; i < 8; ++i) ts.emplace_back([&] { l.arrive_and_wait(); ++past; });
    for (auto &t : ts) t.join(); ts.clear();
    check(past == 8 && l.try_wait(), "std::latch with 8 threads");
    std::atomic<int> phases{0}, badphase{0}, steps{0};
    std::barrier bar(6, [&]() noexcept { ++phases; });
    for (int i = 0; i < 6; ++i) ts.emplace_back([&] { for (int p = 0; p < 20; ++p) { if (phases != p) ++badphase; ++steps; bar.arrive_and_wait(); } });
    for (auto &t : ts) t.join(); ts.clear();
    check(phases == 20 && steps == 120 && badphase == 0, "std::barrier: 20 phases of 6 threads in step");
    std::counting_semaphore<3> sem(3); std::atomic<int> inside{0}, most{0}, rounds{0};
    for (int i = 0; i < 10; ++i) ts.emplace_back([&] {
        for (int k = 0; k < 50; ++k) {
            sem.acquire(); int n = ++inside; int m = most; while (n > m && !most.compare_exchange_weak(m, n)) {}
            std::this_thread::yield(); --inside; ++rounds; sem.release();
        }
    });
    for (auto &t : ts) t.join();
    std::printf("semaphore: %d rounds, at most %d inside\n", rounds.load(), most.load());
    check(rounds == 500 && most <= 3, "std::counting_semaphore<3> with 10 threads");
    std::puts(fails ? "t16 FAILED" : "t16 OK");
    return fails ? 20 : 0;
}
