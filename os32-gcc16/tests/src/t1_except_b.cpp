#include <stdexcept>
struct AppError : std::runtime_error { int code; AppError(const char *m, int c) : std::runtime_error(m), code(c) {} };
void thrower_b(int how)
{
    if (how == 1) throw AppError("library", 1);
    if (how == 2) throw std::logic_error("logic");
    throw 42;
}
