/* Built as C with -fexceptions, so a C++ exception may unwind through it. */
void c_walk(void (*cb)(int), int n) { int i; for (i = 0; i < n; i++) cb(i); }
