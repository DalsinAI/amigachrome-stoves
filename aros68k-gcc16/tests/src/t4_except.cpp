// t4: exceptions. Unwinding through frames, destructors, rethrow, nesting.
#include <cstdio>
#include <stdexcept>
#include <string>
#include <exception>
#include <vector>
#include <atomic>

static int pass, fail, dtors, skipped;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; std::printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

struct Guard { ~Guard() { dtors++; } };
struct MyError : std::runtime_error { int code; MyError(int c) : std::runtime_error("my error"), code(c) {} };

static int deep(int n) { Guard g; if (n == 0) throw MyError(42); return deep(n - 1) + 1; }
struct Throws { Throws() { throw std::logic_error("ctor"); } };

int main()
{
    try { deep(50); CHECK("deep throw", false); }
    catch (const MyError &e) { CHECK("deep throw", e.code == 42 && std::string(e.what()) == "my error"); }
    CHECK("destructors on unwind", dtors == 51);
    try { try { throw 7; } catch (int) { throw; } } catch (int x) { CHECK("rethrow", x == 7); }
    try { throw std::out_of_range("r"); } catch (const std::exception &e) { CHECK("catch by base", std::string(e.what()) == "r"); }
    try { Throws t; } catch (const std::logic_error &) { CHECK("throw from constructor", true); }
#if __GNUC__ >= 7 || ATOMIC_INT_LOCK_FREE > 1   // GCC 6 leaves exception_ptr out without lock-free atomics (68000)
    std::exception_ptr ep;
    try { throw std::runtime_error("saved"); } catch (...) { ep = std::current_exception(); }
    try { std::rethrow_exception(ep); } catch (const std::runtime_error &e) { CHECK("exception_ptr", std::string(e.what()) == "saved"); }
#else
    skipped++; std::printf("  std::exception_ptr not available\n");
#endif
    try { std::vector<int> v; (void)v.at(3); } catch (const std::out_of_range &) { CHECK("library throws", true); }
    int caught = 0; for (int i = 0; i < 1000; i++) { try { if (i % 3 == 0) throw i; } catch (int) { caught++; } }
    CHECK("1000 throws", caught == 334);
    std::printf("t4 exceptions: %d passed, %d failed, %d skipped\n", pass, fail, skipped);
    std::printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
