/* Minimal libc for m68k-elf programs run under qemu-m68k (Linux syscalls). */
#include <stddef.h>
static long sys3(long n, long a, long b, long c)
{
    register long d0 __asm__("d0") = n;
    register long d1 __asm__("d1") = a;
    register long d2 __asm__("d2") = b;
    register long d3 __asm__("d3") = c;
    __asm__ volatile ("trap #0" : "+d"(d0) : "d"(d1), "d"(d2), "d"(d3) : "memory");
    return d0;
}
void exit(int c) { sys3(1, c, 0, 0); for (;;) ; }
void _exit(int c) { exit(c); }
void abort(void) { sys3(4, 2, (long)"abort\n", 6); exit(134); }
long write(int fd, const void *b, size_t n) { return sys3(4, fd, (long)b, (long)n); }
void *memcpy(void *d, const void *s, size_t n) { char *a = d; const char *b = s; while (n--) *a++ = *b++; return d; }
void *memmove(void *d, const void *s, size_t n)
{ char *a = d; const char *b = s; if (a < b) while (n--) *a++ = *b++; else { a += n; b += n; while (n--) *--a = *--b; } return d; }
void *memset(void *d, int c, size_t n) { char *a = d; while (n--) *a++ = (char)c; return d; }
int memcmp(const void *x, const void *y, size_t n)
{ const unsigned char *a = x, *b = y; for (; n; n--, a++, b++) if (*a != *b) return *a - *b; return 0; }
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return (unsigned char)*a - (unsigned char)*b; }
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)) ; return r; }
int puts(const char *s) { write(1, s, strlen(s)); write(1, "\n", 1); return 0; }
int putchar(int c) { char ch = (char)c; write(1, &ch, 1); return c; }
void puthex(const unsigned char *p, size_t n)
{ static const char h[] = "0123456789abcdef"; char b[2]; while (n--) { b[0] = h[*p >> 4]; b[1] = h[*p++ & 15]; write(1, b, 2); } }
static char heap[8 << 20]; static size_t heap_top;
void *malloc(size_t n) { void *p; n = (n + 7) & ~(size_t)7; if (heap_top + n > sizeof heap) return 0; p = heap + heap_top; heap_top += n; return p; }
void *calloc(size_t a, size_t b) { void *p = malloc(a * b); if (p) memset(p, 0, a * b); return p; }
void free(void *p) { (void)p; }
void *realloc(void *p, size_t n) { void *q = malloc(n); if (q && p) memcpy(q, p, n); return q; }
int printf(const char *f, ...) { write(1, f, strlen(f)); return 0; }
int fprintf(void *s, const char *f, ...) { (void)s; write(2, f, strlen(f)); return 0; }
int sprintf(char *d, const char *f, ...) { strcpy(d, f); return (int)strlen(f); }
int strncmp(const char *a, const char *b, size_t n) { for (; n; n--, a++, b++) { if (*a != *b) return (unsigned char)*a - (unsigned char)*b; if (!*a) return 0; } return 0; }
char *strchr(const char *s, int c) { for (;; s++) { if (*s == (char)c) return (char *)s; if (!*s) return 0; } }
int abs(int x) { return x < 0 ? -x : x; }
long labs(long x) { return x < 0 ? -x : x; }
long long llabs(long long x) { return x < 0 ? -x : x; }
void *stdout, *stderr;
int fputs(const char *s, void *f) { (void)f; write(1, s, strlen(s)); return 0; }
