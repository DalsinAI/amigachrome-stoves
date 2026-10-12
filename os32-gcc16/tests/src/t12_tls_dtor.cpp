// SPDX-License-Identifier: MIT
// t12: thread_local objects with constructors and destructors, 5 rounds of 16
// threads. A thread's destructors must have run when join() returns, each
// object must be intact when destroyed, and constructions must equal
// destructions (libpthread's TLS destructor loop at thread exit).
#include <atomic>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
static std::atomic<int> ctors{0}, dtors{0}, ctors2{0}, dtors2{0}, bad{0};
struct Obj {
    unsigned magic = 0x5a5a1234u; int uses = 0; std::string s;
    Obj() : s(64, 'x') { ++ctors; }
    ~Obj() { if (magic != 0x5a5a1234u || s.size() != 64u + uses) ++bad; magic = 0; ++dtors; }
    void use() { ++uses; s += 'y'; }
};
struct Second { int v = 3; Second() { ++ctors2; } ~Second() { if (v != 3) ++bad; ++dtors2; } };
static thread_local Obj obj;
static thread_local Second second;
static thread_local std::vector<int> vec;
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    int early = 0;
    for (int r = 0; r < 5; ++r) {
        std::vector<std::thread> ts;
        for (int t = 0; t < 16; ++t) ts.emplace_back([t] {
            for (int i = 0; i < 10; ++i) obj.use();
            for (int i = 0; i < 100; ++i) vec.push_back(i);
            if (t & 1) second.v = 3;
            if (vec.size() != 100 || obj.uses != 10) ++bad;
        });
        for (auto &t : ts) t.join();
        if (dtors != 16 * (r + 1) || dtors2 != ctors2) ++early;
    }
    std::printf("ctors %d dtors %d, ctors2 %d dtors2 %d, bad %d\n", ctors.load(), dtors.load(), ctors2.load(), dtors2.load(), bad.load());
    check(early == 0, "destructors done when join() returns");
    check(ctors == 80 && dtors == 80, "80 constructions, 80 destructions");
    // GCC constructs all of a file's thread_locals on the first use of any, so
    // Second may be built in all 80 threads; it must be destroyed as often
    check(ctors2 >= 40 && dtors2 == ctors2, "second thread_local: destroyed as often as constructed");
    check(bad == 0, "objects intact in every thread and at destruction");
    bool mainok = obj.uses == 0 && obj.s.size() == 64;   // constructs main's own
    check(mainok && ctors == 81, "main's own object is separate");
    std::puts(fails ? "t12 FAILED" : "t12 OK");
    return fails ? 20 : 0;
}
