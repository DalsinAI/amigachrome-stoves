// t4_atomics: 64-bit atomics and atomic_ref from several threads.
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>
int main()
{
    int fails = 0;
    std::atomic<uint64_t> a{0}; uint64_t plain = 0; std::atomic_ref<uint64_t> r(plain);
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t) ts.emplace_back([&] { for (int i = 0; i < 5000; ++i) { a.fetch_add(0x100000001ULL); r.fetch_add(3); } });
    for (auto &t : ts) t.join();
    if (a.load() != 20000ULL * 0x100000001ULL) { std::puts("FAIL atomic<uint64_t> fetch_add"); ++fails; } else std::puts("PASS atomic<uint64_t> fetch_add");
    if (plain != 60000) { std::puts("FAIL atomic_ref<uint64_t>"); ++fails; } else std::puts("PASS atomic_ref<uint64_t>");
    uint64_t exp = a.load();
    if (!a.compare_exchange_strong(exp, 0xfedcba9876543210ULL) || a.load() != 0xfedcba9876543210ULL) { std::puts("FAIL compare_exchange 64"); ++fails; } else std::puts("PASS compare_exchange 64");
    std::atomic<double> d{1.5}; d.fetch_add(2.25);
    if (d.load() != 3.75) { std::puts("FAIL atomic<double>"); ++fails; } else std::puts("PASS atomic<double>");
    std::printf("lock-free: 32-bit %d, 64-bit %d\n", (int)std::atomic<uint32_t>{}.is_lock_free(), (int)a.is_lock_free());
    std::puts(fails ? "t4 FAILED" : "t4 OK");
    return fails ? 20 : 0;
}
