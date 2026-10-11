/* t10_memmove: an array shifted up by a loop (sntrup761's pattern). GCC 16.2.0b
 * before bebbo's 569d72d67 turned this into a forward copy at -O2 unless
 * -fno-tree-loop-distribute-patterns was given. */
#include <stdio.h>
#define P 761
__attribute__((noinline)) void shift_up(short *v) { int i; for (i = P; i > 0; --i) v[i] = v[i - 1]; v[0] = 0; }
__attribute__((noinline)) void shift_ptr(char *d, char *s, int n) { while (n--) d[n] = s[n]; }
int main(void)
{
    static short v[P + 1]; static char b[64]; int i, bad = 0;
    for (i = 0; i <= P; i++) v[i] = (short)(i * 3 + 1);
    shift_up(v);
    for (i = 1; i <= P; i++) if (v[i] != (short)((i - 1) * 3 + 1)) bad |= 1;
    for (i = 0; i < 64; i++) b[i] = (char)i;
    shift_ptr(b + 5, b, 40);
    for (i = 0; i < 40; i++) if (b[i + 5] != (char)i) bad |= 2;
    puts(bad & 1 ? "FAIL loop shift" : "PASS loop shift");
    puts(bad & 2 ? "FAIL overlapping backward copy" : "PASS overlapping backward copy");
    puts(bad ? "t10 FAILED" : "t10 OK");
    return bad ? 20 : 0;
}
