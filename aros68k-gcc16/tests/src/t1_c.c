/* t1: everyday C and the C library: strings, formatting, sorting, memory. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int pass, fail;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

static int cmp(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y); }

struct packed { char c; int i; short s; double d; };

int main(void)
{
    char buf[128];
    snprintf(buf, sizeof buf, "%d|%5s|%-4d|%x|%o|%c|%%", -42, "ab", 7, 255, 8, 'Z');
    CHECK("snprintf", strcmp(buf, "-42|   ab|7   |ff|10|Z|%") == 0);
    CHECK("strtol", strtol("-0x7fff", NULL, 16) == -32767 && strtoul("4294967295", NULL, 10) == 4294967295UL);
    CHECK("atoi", atoi("  123abc") == 123);
    int v[200];
    unsigned seed = 12345;
    for (int i = 0; i < 200; i++) { seed = seed * 1103515245u + 12345u; v[i] = (int)(seed >> 8) % 10000 - 5000; }
    qsort(v, 200, sizeof v[0], cmp);
    int sorted = 1; for (int i = 1; i < 200; i++) if (v[i - 1] > v[i]) sorted = 0;
    CHECK("qsort", sorted);
    char *p = malloc(100000); CHECK("malloc", p != NULL);
    memset(p, 0xA5, 100000); CHECK("memset", (unsigned char)p[99999] == 0xA5);
    memmove(p + 1, p, 99999); CHECK("memmove", (unsigned char)p[1] == 0xA5);
    p = realloc(p, 200000); CHECK("realloc", p != NULL && (unsigned char)p[50000] == 0xA5);
    free(p);
    CHECK("strings", strlen("hello") == 5 && strncmp("abcd", "abce", 3) == 0 && strchr("x=y", '=')[1] == 'y' && strstr("needle in hay", "in") != NULL);
    CHECK("ctype", toupper('q') == 'Q' && isdigit('7') && !isalpha('7'));
    CHECK("struct layout", sizeof(struct packed) >= 16);
    printf("t1 C library: %d passed, %d failed\n", pass, fail);
    printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
