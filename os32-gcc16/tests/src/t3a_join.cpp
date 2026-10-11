// t3a: four std::threads, a mutex-protected counter, join.
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>
int main()
{
    std::mutex m; long n = 0;
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t) ts.emplace_back([&] { for (int i = 0; i < 1000; ++i) { std::lock_guard<std::mutex> g(m); ++n; } });
    for (auto &t : ts) t.join();
    std::puts(n == 4000 ? "PASS threads, mutex, join" : "FAIL threads, mutex, join");
    std::puts(n == 4000 ? "t3a OK" : "t3a FAILED");
    return n == 4000 ? 0 : 20;
}
