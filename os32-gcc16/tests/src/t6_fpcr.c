/* fpcr.c: GCC 16 -m68040's float-to-int saves FPCR in a scratch data register
 * that may also be the index register of the destination address. */
#define NI __attribute__((noinline, noclone))
NI void store_idx(int *p, int i, double d) { p[i] = (int)d; }
NI void store_idx2(short *p, int i, int j, double d, double e) { p[i + j] = (short)d; p[j] = (short)e; }
NI void tiny(unsigned char *out, const double *in, int n) { int i; for (i = 0; i < n; i++) out[i] = (unsigned char)(int)(in[i] * 0.5 + 64.0); }
NI int sum(int *p, const float *f, int n) { int i, s = 0; for (i = 0; i < n; i++) { p[i * 2 + 1] = (int)f[i]; s += p[i]; } return s; }
volatile double vd = 1234.75;
int main(void)
{
    int a[16] = {0}; short s[16] = {0}; unsigned char o[8]; double in[8]; float f[4] = { 1.5f, -2.5f, 300.9f, 7.0f }; int i, bad = 0;
    store_idx(a, 5, vd); if (a[5] != 1234) bad |= 1;
    for (i = 0; i < 16; i++) if (i != 5 && a[i]) bad |= 2;
    store_idx2(s, 3, 4, vd, -vd); if (s[7] != 1234 || s[4] != -1234) bad |= 4;
    for (i = 0; i < 8; i++) in[i] = i * 10.0;
    tiny(o, in, 8); for (i = 0; i < 8; i++) if (o[i] != (unsigned char)(64 + i * 5)) bad |= 8;
    for (i = 0; i < 16; i++) a[i] = 0;
    sum(a, f, 4); if (a[1] != 1 || a[3] != -2 || a[5] != 300 || a[7] != 7) bad |= 16;
    return bad;
}
