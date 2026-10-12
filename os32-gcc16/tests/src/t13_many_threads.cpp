// SPDX-License-Identifier: MIT
// t13: many threads alive at once, each with thread_local data. libpthread's
// thread table grows 1->4->10->22->46->94->190; the first wave holds 50
// threads at a gate, the second 100. Every thread waits until all of its wave
// has started, so all of them are alive together while the table grows.
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>
static std::atomic<int> ctors{0}, dtors{0};
struct Tl { int v; Tl() : v(7) { ++ctors; } ~Tl() { ++dtors; } };
static thread_local Tl obj;
static thread_local int counter = 1000;
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
static void wave(int n)
{
    std::mutex m; std::condition_variable cv; int arrived = 0; bool go = false;
    std::vector<char> seen(n, 0); std::vector<std::thread> ts; int made = 0;
    int c0 = ctors, d0 = dtors;
    try {
        for (int t = 0; t < n; ++t, ++made) ts.emplace_back([&, t] {
            obj.v += t; counter += t;
            { std::unique_lock<std::mutex> lk(m); if (++arrived == n) { go = true; cv.notify_all(); } cv.wait(lk, [&] { return go; }); }
            std::this_thread::yield();
            seen[t] = obj.v == 7 + t && counter == 1000 + t;
        });
    } catch (const std::system_error &e) {
        std::printf("thread %d of %d not created: %s\n", made + 1, n, e.what());
        std::lock_guard<std::mutex> g(m); go = true; cv.notify_all();
    }
    for (auto &t : ts) t.join();
    int ok = 0; for (char s : seen) ok += s;
    char what[80];
    std::snprintf(what, sizeof what, "%d threads alive at once (%d arrived)", n, arrived); check(arrived == n, what);
    std::snprintf(what, sizeof what, "%d threads: thread_local per thread (%d right)", n, ok); check(ok == n, what);
    std::snprintf(what, sizeof what, "%d threads: %d constructed, %d destroyed", n, ctors - c0, dtors - d0);
    check(ctors - c0 == n && dtors - d0 == n, what);
}
int main()
{
    wave(50);
    wave(100);
    check(counter == 1000, "main's thread_local untouched");
    std::puts(fails ? "t13 FAILED" : "t13 OK");
    return fails ? 20 : 0;
}
