// t3g: four threads allocating and freeing at once, with no lock of our own.
// t3 crashed with every thread feature together; each alone passed. If the C
// library's malloc (libnix, or os32compat's --wrap=malloc) is not thread-safe,
// this corrupts the heap.
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>
int main()
{
    std::vector<std::thread> ts; long sums[4] = {0};
    for (int t = 0; t < 4; ++t) ts.emplace_back([&sums, t] {
        for (int i = 0; i < 3000; ++i) {
            std::string s(16 + (i * 7 + t) % 200, char('a' + t));
            std::vector<int> v((i * 13 + t) % 300 + 1, i);
            void *p = std::malloc(24 + (i % 97)); std::free(p);
            sums[t] += (long)s.size() + (long)v.size();
        }
    });
    for (auto &t : ts) t.join();
    bool ok = sums[0] && sums[1] && sums[2] && sums[3];
    std::puts(ok ? "PASS malloc and new in four threads at once" : "FAIL malloc and new in four threads at once");
    std::puts(ok ? "t3g OK" : "t3g FAILED");
    return ok ? 0 : 20;
}
