// SPDX-License-Identifier: MIT
// t14: std::shared_mutex with 6 readers and 2 writers (readers must never see
// a half-done write), and std::shared_timed_mutex's try_lock_for and
// try_lock_shared while another thread holds it shared.
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    std::shared_mutex sm; long a = 0, b = 0;
    std::atomic<int> torn{0}, reads{0}, inside{0}, maxinside{0};
    std::vector<std::thread> ts;
    for (int w = 0; w < 2; ++w) ts.emplace_back([&] {
        for (int i = 0; i < 300; ++i) {
            std::unique_lock<std::shared_mutex> lk(sm);
            ++a; for (volatile int k = 0; k < 50; ++k) {} ++b;
        }
    });
    for (int r = 0; r < 6; ++r) ts.emplace_back([&] {
        for (int i = 0; i < 400; ++i) {
            std::shared_lock<std::shared_mutex> lk(sm);
            int n = ++inside; int m = maxinside; while (n > m && !maxinside.compare_exchange_weak(m, n)) {}
            if (a != b) ++torn;
            ++reads; --inside;
        }
    });
    for (auto &t : ts) t.join();
    std::printf("a %ld b %ld, reads %d, torn %d, most readers inside at once %d\n", a, b, reads.load(), torn.load(), maxinside.load());
    check(a == 600 && b == 600, "writers: 600 exclusive updates");
    check(reads == 2400 && torn == 0, "readers: 2400 reads, none saw a half-done write");
    std::shared_timed_mutex stm; std::atomic<bool> held{false}, release{false};
    std::thread holder([&] { std::shared_lock<std::shared_timed_mutex> lk(stm); held = true; while (!release) std::this_thread::yield(); });
    while (!held) std::this_thread::yield();
    auto t0 = std::chrono::steady_clock::now();
    bool got = stm.try_lock_for(std::chrono::milliseconds(100));
    long ms = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    if (got) stm.unlock();
    bool shared = stm.try_lock_shared(); if (shared) stm.unlock_shared();
    release = true; holder.join();
    std::printf("try_lock_for gave up after %ld ms\n", ms);
    check(!got && ms >= 80, "shared_timed_mutex: try_lock_for times out while held shared");
    check(shared, "shared_timed_mutex: try_lock_shared while held shared");
    bool excl = stm.try_lock(); if (excl) stm.unlock();
    check(excl, "shared_timed_mutex: try_lock once free");
    std::puts(fails ? "t14 FAILED" : "t14 OK");
    return fails ? 20 : 0;
}
