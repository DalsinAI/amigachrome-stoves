#include <cstdio>
static int calls;
static int make() { return ++calls; }
static int &once() { static int v = make(); return v; }   // a static-init guard (__cxa_guard_acquire)
int main() { once(); once(); std::printf("h3 static guard: %d\n", calls); return 0; }
