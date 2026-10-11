/* t64.c: 64-bit value patterns, printed so the host and m68k builds can be
 * compared line by line. Arguments on the stack, spills, shifts, rotates,
 * widening multiplies, structs and varargs. */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#ifdef HOST
#include <stdio.h>
static void puthex(const unsigned char *p, size_t n) { while (n--) printf("%02x", *p++); }
static void say(const char *s) { fputs(s, stdout); }
#else
void puthex(const unsigned char *p, size_t n);
long write(int fd, const void *b, size_t n);
static void say(const char *s) { write(1, s, strlen(s)); }
#endif

#define NI __attribute__((noinline, noclone))
static void out64(const char *n, uint64_t v)
{ unsigned char b[8]; int i; for (i = 0; i < 8; i++) b[i] = (unsigned char)(v >> (56 - 8 * i)); say(n); say(" "); puthex(b, 8); say("\n"); }

volatile uint64_t vx = 0x0123456789abcdefULL, vy = 0xfedcba9876543210ULL;
volatile uint32_t va = 0xdeadbeef, vb = 0x12345678;

NI uint64_t add6(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f) { return a + b * 3 + (c ^ d) - (e >> 7) + (f << 3); }
NI uint64_t mix(uint32_t x, uint64_t a, uint16_t y, uint64_t b, uint8_t z, uint64_t c) { return (a ^ (b >> 13)) + c * x + y - z; }
NI uint64_t mul32(uint32_t a, uint32_t b) { return (uint64_t)a * b; }
NI int64_t smul32(int32_t a, int32_t b) { return (int64_t)a * b; }
NI uint64_t mul64(uint64_t a, uint64_t b) { return a * b; }
NI uint64_t div64(uint64_t a, uint64_t b) { return a / b + a % b; }
NI int64_t sdiv64(int64_t a, int64_t b) { return a / b - a % b; }
NI uint64_t rotr(uint64_t x, int n) { return (x >> n) | (x << (64 - n)); }
NI uint64_t sigma(uint64_t x) { return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39) ^ (x >> 7) ^ (x << 9) ^ (x >> 32) ^ (x << 32) ^ (x >> 16) ^ (x << 8) ^ (x >> 1); }
NI uint32_t hi(uint64_t x) { return (uint32_t)(x >> 32); }
NI uint32_t lo(uint64_t x) { return (uint32_t)x; }
NI uint64_t sumv(int n, ...) { va_list ap; uint64_t s = 0; va_start(ap, n); while (n--) s += va_arg(ap, uint64_t) * (uint64_t)(n + 1); va_end(ap); return s; }
struct S { uint32_t a; uint64_t b; uint16_t c; uint64_t d; };
NI uint64_t sfn(struct S s, uint64_t k) { return s.a + s.b * k + s.c + (s.d >> 3); }
NI struct S sret(uint64_t a, uint64_t b) { struct S s; s.a = (uint32_t)a; s.b = a ^ b; s.c = (uint16_t)b; s.d = b - a; return s; }

/* register pressure: many live 64-bit values, so some spill to the stack */
NI uint64_t pressure(const uint64_t *w, int n)
{
    uint64_t a = w[0], b = w[1], c = w[2], d = w[3], e = w[4], f = w[5], g = w[6], h = w[7];
    int i;
    for (i = 0; i < n; i++) {
        uint64_t t1 = h + (rotr(e, 14) ^ rotr(e, 18) ^ rotr(e, 41)) + ((e & f) ^ (~e & g)) + w[i & 7];
        uint64_t t2 = (rotr(a, 28) ^ rotr(a, 34) ^ rotr(a, 39)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    return a ^ b ^ c ^ d ^ e ^ f ^ g ^ h;
}

/* 32x32->64 accumulations as bignum code does (i31/i15-style, ghash, poly1305) */
NI uint64_t macc(const uint32_t *x, const uint32_t *y, int n)
{
    uint64_t acc = 0, cc = 0; int i;
    for (i = 0; i < n; i++) { uint64_t z = (uint64_t)x[i] * y[n - 1 - i] + cc; acc += z; cc = z >> 31; }
    return acc ^ (cc << 17);
}

NI int cmp64(int64_t a, int64_t b) { return (a < b) * 4 + (a == b) * 2 + (a > b); }
NI uint64_t sel(uint64_t a, uint64_t b, int c) { return c ? a : b; }
NI uint64_t shl_var(uint64_t a, int n) { return a << n; }
NI uint64_t shr_var(uint64_t a, int n) { return a >> n; }
NI int64_t sar_var(int64_t a, int n) { return a >> n; }
NI uint64_t bswap(uint64_t x) { return __builtin_bswap64(x); }
NI int clz(uint64_t x) { return __builtin_clzll(x | 1); }
NI int ctz(uint64_t x) { return __builtin_ctzll(x | 0x8000000000000000ULL); }
NI uint64_t neg(uint64_t x) { return -x; }
NI uint64_t from_bytes(const unsigned char *p) { uint64_t v = 0; int i; for (i = 0; i < 8; i++) v = (v << 8) | p[i]; return v; }
NI void to_bytes(unsigned char *p, uint64_t v) { int i; for (i = 7; i >= 0; i--) { p[i] = (unsigned char)v; v >>= 8; } }
NI double u2d(uint64_t x) { return (double)x; }
NI uint64_t d2u(double d) { return (uint64_t)d; }
NI int64_t f2s(float f) { return (int64_t)f; }

int main(void)
{
    uint64_t x = vx, y = vy, w[8]; uint32_t a = va, b = vb; int i;
    unsigned char buf[8];
    for (i = 0; i < 8; i++) w[i] = x * (i + 1) ^ (y >> i);
    out64("add6", add6(x, y, x + 1, y + 2, x ^ y, y - x));
    out64("mix", mix(a, x, 0xbeef, y, 0x5a, x ^ 0x55));
    out64("mul32", mul32(a, b)); out64("smul32", (uint64_t)smul32((int32_t)a, (int32_t)b));
    out64("mul64", mul64(x, y)); out64("div64", div64(y, x >> 7)); out64("sdiv64", (uint64_t)sdiv64(-(int64_t)y, 12345));
    out64("rotr", rotr(x, 19)); out64("sigma", sigma(y));
    out64("hilo", ((uint64_t)hi(x) << 32) | lo(y));
    out64("sumv", sumv(5, x, y, x * 3, y * 5, x ^ y));
    { struct S s = sret(x, y); out64("sret", s.a ^ s.b ^ s.c ^ s.d); out64("sfn", sfn(s, y)); }
    out64("pressure", pressure(w, 80));
    { uint32_t X[16], Y[16]; for (i = 0; i < 16; i++) { X[i] = (uint32_t)(w[i & 7] >> (i & 3)); Y[i] = (uint32_t)(w[(i + 3) & 7] >> 11) & 0x7fffffff; }
      out64("macc", macc(X, Y, 16)); }
    out64("cmp", (uint64_t)(cmp64((int64_t)x, (int64_t)y) * 100 + cmp64((int64_t)y, (int64_t)y) * 10 + cmp64(-1, 0)));
    out64("sel", sel(x, y, a & 1) + sel(x, y, b & 1));
    out64("shl", shl_var(x, 37) ^ shl_var(y, 3)); out64("shr", shr_var(y, 33) ^ shr_var(x, 5)); out64("sar", (uint64_t)(sar_var(-(int64_t)x, 40) ^ sar_var((int64_t)y, 2)));
    out64("bswap", bswap(x)); out64("clzctz", (uint64_t)(clz(x >> 9) * 100 + ctz(y << 5)));
    out64("neg", neg(x));
    to_bytes(buf, y); out64("bytes", from_bytes(buf) ^ x);
    out64("u2d", (uint64_t)(u2d(x >> 11) / 3.0)); out64("d2u", d2u(1.5e18)); out64("f2s", (uint64_t)f2s(-3.25e12f));
    say("end\n");
    return 0;
}
