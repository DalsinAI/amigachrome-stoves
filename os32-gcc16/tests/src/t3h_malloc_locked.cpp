// t3h: the same as t3g, every allocation under one mutex (the control).



#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
static std::mutex heap;
int main()
{
    std::vector<std::thread> ts; long sums[4] = {0};
    for (int t = 0; t < 4; ++t) ts.emplace_back([&sums, t] {
        for (int i = 0; i < 3000; ++i) {
            std::lock_guard<std::mutex> g(heap);
            std::string s(16 + (i * 7 + t) % 200, char('a' + t));
            std::vector<int> v((i * 13 + t) % 300 + 1, i);
            void *p = std::malloc(24 + (i % 97)); std::free(p);
            sums[t] += (long)s.size() + (long)v.size();
        }
    });
    for (auto &t : ts) t.join();
    bool ok = sums[0] && sums[1] && sums[2] && sums[3];
    std::puts(ok ? "PASS malloc and new in four threads under one mutex" : "FAIL malloc and new in four threads under one mutex");
    std::puts(ok ? "t3h OK" : "t3h FAILED");
    return ok ? 0 : 20;
}
