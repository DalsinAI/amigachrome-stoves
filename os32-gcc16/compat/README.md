# libos32compat for the os32-gcc16 stove

What libstdc++ (GCC 16) and C++ programs need that libnix does not have:
`__xpg_strerror_r` and other C symbols (`os32compat.c`), `aligned_alloc`/`memalign`/`posix_memalign`
over a wrapped malloc (`os32compat_malloc.c`), C99 float functions (`os32compat_math.c`),
C++-mangled `clock_*` and `statvfs` (`os32compat_cxx.cpp`, `os32compat_statvfs.cpp`), and
long double maths through double (`ldbl/`). `fenv.h` and `uchar.h` are header-only.

    sh build.sh [OUT_DIR]        STOVE defaults to ~/AmigaChrome/stoves/os32-gcc16

`build.sh` makes three libraries: `libos32compat.a` (absolute), `libb/libos32compat.a` (`-fbaserel`) and `libb32/libos32compat.a` (`-fbaserel32`). Base-relative programs need their own build: the absolute one uses a4 as a scratch register in the malloc wrappers, and a `-fbaserel32` program linked with it hangs. The absolute one reproduces the stove's installed `compat/libos32compat.a` object for object
(the same disassembly for all 64 members, checked against the stove installed on daletop).
Link it after libstdc++: `-lstdc++ -L<stove>/compat -los32compat`, with
`-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free` for the aligned allocator.
