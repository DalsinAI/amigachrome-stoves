# libos32compat for the os32-gcc16 stove

What libstdc++ (GCC 16) and C++ programs need that libnix does not have:
`__xpg_strerror_r` and other C symbols (`os32compat.c`), `aligned_alloc`/`memalign`/`posix_memalign`
over a wrapped malloc (`os32compat_malloc.c`), C99 float functions (`os32compat_math.c`),
C++-mangled `clock_*` and `statvfs` (`os32compat_cxx.cpp`, `os32compat_statvfs.cpp`), and
long double maths through double (`ldbl/`). `fenv.h` and `uchar.h` are header-only.

    sh build.sh [OUT_DIR]        STOVE defaults to ~/AmigaChrome/stoves/os32-gcc16

`build.sh` reproduces the stove's installed `compat/libos32compat.a` object for object
(the same disassembly for all 64 members, checked against the stove installed on daletop).
Link it after libstdc++: `-lstdc++ -L<stove>/compat -los32compat`, with
`-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free` for the aligned allocator.
