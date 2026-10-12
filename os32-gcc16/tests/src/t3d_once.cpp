// t3d: std::call_once from four threads (libstdc++ calls through a thread-local pointer).
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>
static std::once_flag once; static int count;
int main()
{
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t) ts.emplace_back([] { std::call_once(once, [] { ++count; }); });
    for (auto &t : ts) t.join();
    std::call_once(once, [] { ++count; });
    std::puts(count == 1 ? "PASS call_once" : "FAIL call_once");
    std::puts(count == 1 ? "t3d OK" : "t3d FAILED");
    return count == 1 ? 0 : 20;
}
