// t3c: thread_local (emulated TLS on AmigaOS) in four threads.
#include <cstdio>
#include <thread>
#include <vector>
static thread_local int tl = 100;
int main()
{
    int seen[4] = {0}; std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t) ts.emplace_back([&seen, t] { tl += t + 1; seen[t] = tl; });
    for (auto &t : ts) t.join();
    bool ok = tl == 100 && seen[0] == 101 && seen[1] == 102 && seen[2] == 103 && seen[3] == 104;
    std::puts(ok ? "PASS thread_local" : "FAIL thread_local");
    std::puts(ok ? "t3c OK" : "t3c FAILED");
    return ok ? 0 : 20;
}
