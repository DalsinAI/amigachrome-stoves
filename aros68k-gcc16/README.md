# aros68k-gcc16: GCC 16.2 for AROS 68k

A modern C and C++ compiler for AROS on the 68k: GCC 16.2.0 with binutils
2.47, built through AROS's own crosstools from AROS
[`c8860e674d`](https://github.com/aros-development-team/AROS/commit/c8860e674d7c5fbab304e2bf201f7a9c81c98629),
whose patches cover m68k-aros up to GCC 16.2. AROS still builds 68k with GCC
6.5 by default; this stove sits beside it and replaces nothing.

**Status: experimental.** First built 2 October 2026. The test results are
below once they are in.

## What it gives you

- C23 by default, C++20 and later, and a current libstdc++.
- `std::exception_ptr` on the 68000. GCC 6.5's library leaves it out on CPUs
  without lock-free atomics.
- The same AROS headers and libraries as an AROS build, as its sysroot.
- Programs are AROS 68k programs (ELF). They need AROS, not AmigaOS.
- The default CPU is a 68000 with software floating point, as AROS 68k's is.
  `-m68020`, `-m68040` and `-m68881` are there for faster machines.

## Build it

```sh
./build.sh                 # about an hour on 8 cores
./build.sh --verify-only   # just fetch-check the sources
```

`sources.lock` pins every input by SHA-256. GCC and binutils are also checked
against their GNU signatures with `gpgv`. One quirk handled for you: AROS
fetches isl as `.tar.bz2` while isl publishes `.tar.xz`, so the script repacks
it.

The result is in `work/`:

| Folder | What |
| --- | --- |
| `work/toolchain/` | the compiler: `m68k-aros-gcc`, `m68k-aros-g++` and friends |
| `work/sdk-build/` | the AROS crosstools build; it holds the compiler's sysroot, so keep it |
| `work/stove.json` | the identity: versions, AROS ref, build time |

Check which compiler you have with `m68k-aros-gcc --version`: this stove says
16.2.0.

## Use it

```sh
work/toolchain/m68k-aros-gcc -O2 -o hello hello.c
work/toolchain/m68k-aros-g++ -O2 -std=gnu++20 -o app app.cpp
work/toolchain/m68k-aros-g++ -O2 -o threads threads.cpp -lpthread -latomic
```

Old Amiga sources may need `-std=gnu11 -fpermissive`. Since GCC 14, implicit
function declarations, implicit int and pointer/integer mixing are errors;
since GCC 15, C23 is the default, so `bool` is a keyword and `int f()` means
no arguments. On the Amiga, address 0 is real memory, and code that bends the
aliasing rules is common, so `-fno-delete-null-pointer-checks` and
`-fno-strict-aliasing` are worth having for old code.

## Tests

`tests/` holds seven programs, built by this stove and, for comparison, by
AROS's GCC 6.5:

| Test | Covers |
| --- | --- |
| t1 | the C library: formatting, strings, sorting, memory |
| t2 | 64-bit integers and floating point (soft-float on a 68000) |
| t3 | C++: classes, containers, lambdas; C++17 and C++20 when the compiler has them |
| t4 | exceptions: deep unwinding, destructors, rethrow, `exception_ptr` |
| t5 | threads: pthreads, `std::thread`, mutex, condition variable, atomics |
| t6 | the AROS API: exec and dos calls through the library stubs |
| t7 | a benchmark whose checksums must match a host build's |

```sh
G6=/path/to/aros/toolchain/m68k-aros- tests/build-tests.sh   # G6 is optional
```

Then copy `tests/bin/` and `tests/run-tests` to `DH1:GCC16/` on an AROS 68k
machine and `Execute DH1:GCC16/run-tests`. Results land in
`DH1:GCC16/results.txt`.

## Results

Pending: the first run is under way.

## Findings so far

- GCC 6.5 (AROS's default) has no `std::exception_ptr` on the 68000, and
  `std::atomic` needs `-latomic`. Both are in the tests.
