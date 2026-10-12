// SPDX-License-Identifier: MIT
// t19: 1150 short threads, each touching a thread_local with a destructor: 600
// created and joined one after another, 400 in batches of 8, 50 detached, and
// 100 std::async tasks. libpthread must reuse its slots, detach must work, and
// every thread_local must be destroyed.
#include <atomic>
#include <chrono>
#include <cstdio>
#include <future>
#include <string>
#include <system_error>
#include <thread>
#include <vector>
static std::atomic<int> ctors{0}, dtors{0};
struct Tl { std::vector<int> v; Tl() : v(16, 1) { ++ctors; } ~Tl() { ++dtors; } };
static thread_local Tl tl;
static thread_local int tli = 5;
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
static int work() { int r = (int)tl.v.size() + tli; ++tli; return r; }
int main()
{
    std::atomic<long> sum{0};
    for (int i = 0; i < 600; ++i) { std::thread t([&] { sum += work(); }); t.join(); }
    check(sum == 600 * 21 && ctors == 600, "600 threads one after another");
    for (int b = 0; b < 50; ++b) {
        std::vector<std::thread> ts;
        for (int i = 0; i < 8; ++i) ts.emplace_back([&] { sum += work(); });
        for (auto &t : ts) t.join();
    }
    check(sum == 1000 * 21 && ctors == 1000, "400 more in 50 batches of 8");
    std::atomic<int> ran{0}; int detached = 0; std::string why;
    for (int i = 0; i < 50; ++i) {
        std::thread t([&] { sum += work(); ++ran; });
        try { t.detach(); ++detached; } catch (const std::system_error &e) { if (why.empty()) why = e.what(); t.join(); }
    }
    if (!why.empty()) std::printf("std::thread::detach threw: %s\n", why.c_str());
    for (int k = 0; k < 500 && ran < 50; ++k) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    check(detached == 50 && ran == 50, "50 detached threads");
    int got = 0;
    for (int i = 0; i < 100; ++i) got += std::async(std::launch::async, work).get();
    check(got == 100 * 21 && ctors == 1150, "100 std::async tasks");
    for (int k = 0; k < 100 && dtors < ctors; ++k) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    char what[80]; std::snprintf(what, sizeof what, "thread_local destroyed in every thread (%d of %d)", dtors.load(), ctors.load());
    check(dtors == ctors && ctors == 1150, what);
    std::puts(fails ? "t19 FAILED" : "t19 OK");
    return fails ? 20 : 0;
}
