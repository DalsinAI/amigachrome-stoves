// t1_except: C++ exceptions across translation units, a static library and a
// C callback, with destructors run on the way. Prints PASS or FAIL lines.
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string>

struct Counted { static int live; Counted() { ++live; } ~Counted() { --live; } };
int Counted::live = 0;

struct AppError : std::runtime_error { int code; AppError(const char *m, int c) : std::runtime_error(m), code(c) {} };

void thrower_b(int how);                        // t1_except_b.cpp, built into libt1.a
extern "C" void c_walk(void (*cb)(int), int n); // t1_except_c.c, built with -fexceptions

static int fails;
static void check(bool ok, const char *what) { std::printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++fails; }

static void cb(int i) { Counted c; if (i == 3) throw AppError("from callback", 3); }

int main()
{
    try { Counted a; thrower_b(1); check(false, "throw from library"); }
    catch (const AppError &e) { check(e.code == 1 && Counted::live == 0, "catch derived from library, destructors ran"); }

    try { thrower_b(2); }
    catch (const std::exception &e) { check(std::string(e.what()) == "logic", "catch by base class"); }

    try { try { thrower_b(1); } catch (...) { throw; } }
    catch (const AppError &e) { check(e.code == 1, "rethrow"); }

    std::exception_ptr p;
    try { thrower_b(3); } catch (...) { p = std::current_exception(); }
    try { if (p) std::rethrow_exception(p); check(false, "exception_ptr"); }
    catch (int v) { check(v == 42, "exception_ptr carries an int"); }

    try { c_walk(cb, 10); check(false, "through C"); }
    catch (const AppError &e) { check(e.code == 3 && Counted::live == 0, "through a C function built with -fexceptions"); }

    try { throw std::string(4000, 'x'); }
    catch (const std::string &s) { check(s.size() == 4000, "large exception object"); }

    std::printf("%s\n", fails ? "t1 FAILED" : "t1 OK");
    return fails ? 20 : 0;
}
