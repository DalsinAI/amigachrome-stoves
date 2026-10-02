/* t2: 64-bit integers and floating point (soft-float on a 68000). */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

static int pass, fail;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

int main(void)
{
    volatile int64_t a = -1234567890123LL, b = 98765;
    CHECK("i64 mul", a * b == -121932097667998095LL);
    CHECK("i64 div", a / b == -12500054LL && a % b == -56813LL);
    volatile uint64_t u = 0xFEDCBA9876543210ULL;
    CHECK("u64 shift", (u >> 36) == 0xFEDCBA9ULL && (u << 20) == 0xA987654321000000ULL && (uint32_t)(u >> 32) == 0xFEDCBA98u);
    CHECK("u64 div", u / 1000000007ULL == 18364758415ULL && u % 1000000007ULL == 939755815ULL);
    volatile int32_t m = -7, n = 2;
    CHECK("i32 div", m / n == -3 && m % n == -1);
    volatile double x = 1.0 / 3.0, y = 2.5;
    CHECK("double basic", x * 3.0 == 1.0 && y * y == 6.25 && (int)(y * 4) == 10);
    CHECK("sqrt", sqrt(2.0) == 1.4142135623730951);
    CHECK("pow", pow(2.0, 10.0) == 1024.0);
    CHECK("sin", fabs(sin(1.0) - 0.8414709848078965) < 1e-15);
    CHECK("exp/log", fabs(exp(log(10.0)) - 10.0) < 1e-13);
    volatile float f = 16777216.0f;
    CHECK("float", f + 1.0f == 16777216.0f && (double)(float)0.1 != 0.1);
    CHECK("conversion", (int64_t)-2.75 == -2 && (uint32_t)4000000000.0 == 4000000000u && (double)(int64_t)9007199254740993LL == 9007199254740992.0);
    char buf[64];
    snprintf(buf, sizeof buf, "%.10f %e %g", 3.14159265358979, 12345.678, 0.0001);
    printf("  formatted: %s\n", buf);
    printf("  i64 product: %lld\n", (long long)(a * b));
    printf("t2 math: %d passed, %d failed\n", pass, fail);
    printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
