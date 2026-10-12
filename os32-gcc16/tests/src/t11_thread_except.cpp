// SPDX-License-Identifier: MIT
// t11: exceptions and threads. Thrown and caught inside 8 threads at once;
// carried out of std::async, std::promise, std::packaged_task and an
// exception_ptr into the thread that waits for them.
#include <atomic>
#include <cstdio>
#include <exception>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
struct Err : std::runtime_error { int n; explicit Err(int n) : std::runtime_error("err " + std::to_string(n)), n(n) {} };
static int deep(int d, int n) { if (d == 0) throw Err(n); return deep(d - 1, n) + 1; }
static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }
int main()
{
    std::atomic<int> caught{0}, wrong{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < 8; ++t) ts.emplace_back([&caught, &wrong, t] {
        for (int i = 0; i < 200; ++i) {
            int n = t * 1000 + i;
            try { deep(1 + (i + t) % 12, n); ++wrong; }
            catch (const Err &e) { if (e.n == n && e.what() == "err " + std::to_string(n)) ++caught; else ++wrong; }
            catch (...) { ++wrong; }
        }
    });
    for (auto &t : ts) t.join();
    std::printf("caught %d wrong %d\n", caught.load(), wrong.load());
    check(caught == 1600 && wrong == 0, "thrown and caught inside 8 threads");
    int got = 0;
    for (int i = 0; i < 20; ++i) {
        auto f = std::async(std::launch::async, [i] { return deep(5, i); });
        try { f.get(); } catch (const Err &e) { if (e.n == i) ++got; } catch (...) {}
    }
    check(got == 20, "std::async: future::get rethrows (20 tasks)");
    std::promise<int> p; auto pf = p.get_future();
    std::thread pt([&p] { try { throw std::logic_error("promised"); } catch (...) { p.set_exception(std::current_exception()); } });
    bool pok = false;
    try { pf.get(); } catch (const std::logic_error &e) { pok = std::string(e.what()) == "promised"; } catch (...) {}
    pt.join();
    check(pok, "std::promise::set_exception from a thread");
    std::packaged_task<int(int)> task([](int n) -> int { if (n > 0) throw Err(n); return 0; });
    auto tf = task.get_future(); std::thread tt(std::move(task), 9); tt.join();
    bool tok = false;
    try { tf.get(); } catch (const Err &e) { tok = e.n == 9; } catch (...) {}
    check(tok, "std::packaged_task in a thread");
    std::exception_ptr ep;
    std::thread et([&ep] { try { deep(3, 77); } catch (...) { ep = std::current_exception(); } });
    et.join();
    bool eok = false;
    try { if (ep) std::rethrow_exception(ep); } catch (const Err &e) { eok = e.n == 77; } catch (...) {}
    check(eok, "exception_ptr rethrown in main");
    std::puts(fails ? "t11 FAILED" : "t11 OK");
    return fails ? 20 : 0;
}
