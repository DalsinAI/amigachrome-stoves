/* t7: the same work for both compilers; the checksums must match, the times
   compare the code each compiler makes. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static uint32_t crc_table[256];
static uint32_t crc32_buf(const uint8_t *p, size_t n)
{
    uint32_t c = 0xFFFFFFFFu;
    while (n--) c = crc_table[(c ^ *p++) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}
static void my_qsort(int *a, int lo, int hi)
{
    while (lo < hi) {
        int p = a[(lo + hi) / 2], i = lo, j = hi;
        while (i <= j) { while (a[i] < p) i++; while (a[j] > p) j--; if (i <= j) { int t = a[i]; a[i] = a[j]; a[j] = t; i++; j--; } }
        if (j - lo < hi - i) { my_qsort(a, lo, j); lo = i; } else { my_qsort(a, i, hi); hi = j; }
    }
}
static double ms(clock_t a, clock_t b) { return (double)(b - a) * 1000.0 / CLOCKS_PER_SEC; }

int main(void)
{
    for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1; crc_table[i] = c; }
    static uint8_t buf[262144];
    for (size_t i = 0; i < sizeof buf; i++) buf[i] = (uint8_t)(i * 31 + (i >> 7));
    clock_t t0 = clock(); uint32_t crc = 0;
    for (int r = 0; r < 8; r++) { buf[r] ^= 1; crc ^= crc32_buf(buf, sizeof buf); }
    clock_t t1 = clock();
    static char sieve[400000]; int primes = 0;
    for (int r = 0; r < 4; r++) { memset(sieve, 1, sizeof sieve); primes = 0; for (int i = 2; i < (int)sizeof sieve; i++) if (sieve[i]) { primes++; for (int j = i * 2; j < (int)sizeof sieve; j += i) sieve[j] = 0; } }
    clock_t t2 = clock();
    static int arr[60000]; uint32_t s = 7; long long sorted_sum = 0;
    for (int r = 0; r < 3; r++) { for (int i = 0; i < 60000; i++) { s = s * 1664525u + 1013904223u; arr[i] = (int)(s >> 4); } my_qsort(arr, 0, 59999); sorted_sum += arr[0] + arr[30000] + arr[59999]; }
    clock_t t3 = clock();
    static int A[64][64], B[64][64], C[64][64]; long long msum = 0;
    for (int i = 0; i < 64; i++) for (int j = 0; j < 64; j++) { A[i][j] = (i * 3 + j) % 17 - 8; B[i][j] = (i + j * 5) % 13 - 6; }
    for (int r = 0; r < 4; r++) { for (int i = 0; i < 64; i++) for (int j = 0; j < 64; j++) { int acc = 0; for (int k = 0; k < 64; k++) acc += A[i][k] * B[k][j]; C[i][j] = acc + r; } msum += C[r][63 - r]; }
    clock_t t4 = clock();
    int inside = 0;   /* soft-float on a 68000 */
    for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) { double cr = -2.0 + x * 3.0 / 64, ci = -1.2 + y * 2.4 / 48, zr = 0, zi = 0; int n = 0; while (n < 64 && zr * zr + zi * zi < 4.0) { double t = zr * zr - zi * zi + cr; zi = 2 * zr * zi + ci; zr = t; n++; } if (n == 64) inside++; }
    clock_t t5 = clock();
    printf("  crc32   %08lx  %8.0f ms\n", (unsigned long)crc, ms(t0, t1));
    printf("  sieve   %8d  %8.0f ms\n", primes, ms(t1, t2));
    printf("  qsort   %lld  %8.0f ms\n", sorted_sum, ms(t2, t3));
    printf("  matrix  %lld  %8.0f ms\n", msum, ms(t3, t4));
    printf("  mandel  %8d  %8.0f ms\n", inside, ms(t4, t5));
    printf("  total            %8.0f ms\n", ms(t0, t5));
    int ok = crc == 0xc80a0dccu && primes == 33860 && sorted_sum == 1206985858LL && msum == -73 && inside == 688;
    printf("t7 bench: checksums %s\n", ok ? "match" : "DIFFER");
    printf("%s\n", ok ? "RESULT PASS" : "RESULT FAIL");
    return ok ? 0 : 10;
}
