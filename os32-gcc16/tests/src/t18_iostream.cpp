// SPDX-License-Identifier: MIT
// t18: iostreams from 8 threads at once. Each formats with its own
// ostringstream (ints, hex, fixed, setw) and parses with istringstream, which
// shares the global locale and its facets; lines go to std::cout under a
// mutex. Last, 4 threads write whole lines to std::cout with no lock (allowed
// by the standard for a synchronised stream; lines may interleave, but the
// program must not crash).
#include <atomic>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); std::fflush(stdout); if (!ok) ++fails; }
int main()
{
    std::mutex om; std::atomic<int> bad{0}, done{0}; std::vector<std::thread> ts;
    for (int t = 0; t < 8; ++t) ts.emplace_back([&, t] {
        for (int i = 0; i < 300; ++i) {
            std::ostringstream os;
            os << "t" << t << " i=" << std::setw(4) << i << " h=" << std::hex << i * (t + 1) << std::dec
               << " f=" << std::fixed << std::setprecision(2) << i / 4.0;
            char want[80]; std::snprintf(want, sizeof want, "t%d i=%4d h=%x f=%.2f", t, i, i * (t + 1), i / 4.0);
            if (os.str() != want) ++bad;
            std::istringstream is(std::to_string(i) + " " + std::to_string(t) + ".5 word" + std::to_string(t));
            int a = -1; double d = 0; std::string w; is >> a >> d >> w;
            if (a != i || d != t + 0.5 || w != "word" + std::to_string(t) || std::stoi(std::to_string(i * 3)) != i * 3) ++bad;
            if (i % 100 == 0) { std::lock_guard<std::mutex> g(om); std::cout << "thread " << t << " at " << i << ": " << os.str() << std::endl; }
        }
        ++done;
    });
    for (auto &t : ts) t.join(); ts.clear();
    std::printf("%d threads done, %d mismatches\n", done.load(), bad.load());
    check(done == 8 && bad == 0, "ostringstream/istringstream in 8 threads: 2400 right");
    for (int t = 0; t < 4; ++t) ts.emplace_back([t] { for (int i = 0; i < 5; ++i) std::cout << ("unlocked line from thread " + std::to_string(t) + "\n"); });
    for (auto &t : ts) t.join();
    std::cout.flush();
    check(std::cout.good(), "std::cout unlocked from 4 threads: still good");
    std::puts(fails ? "t18 FAILED" : "t18 OK");
    return fails ? 20 : 0;
}
