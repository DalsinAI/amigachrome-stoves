# os32-gcc16: tests and patches

The os32-gcc16 stove is bebbo's amiga-gcc with GCC branch `amiga16.2`
(GCC 16.2.0b) for AmigaOS 3.x. Its recipe and the first three patches are in
openamigabrowser's `stove/` (`STOVE-NOTES.txt`, `gcc-patches/0001`–`0003`). The
built stove stays on the machines that hold the NDK. This folder holds what
does not need the NDK:

- `patches/0004-m68k-fix-trunc-68040-scratch-earlyclobber.patch`: the fix for
  GCC 16's 68040 FPCR clash (below). GPL, like GCC.
- `tests/`: qualification programs for the stove, built with
  `tests/build-tests.sh`, run on an instance with `tests/run-tests` and judged
  with `tests/check-results.sh`. MIT.
- `tests/qemu-check/`: how the compilers were checked on 11 October 2026
  without an Amiga (upstream GCC 16.2.0 and bebbo's amiga16.2 under
  qemu-m68k). MIT.

## Known GCC 16 problems on the 68k (11 October 2026)

| Problem | Seen in | State | How it is checked |
| --- | --- | --- | --- |
| **68040 FPCR clash.** `-m68040` float-to-integer saves FPCR in a scratch register that can also be the index register of the store's address: `fmovem.l fpcr,d0 … fmove.l fp0,(a0,d0.l*4)` | bebbo amiga16.2 (7 and 11 Oct); upstream 16.2.0 with `-mlra` | **Fixed by patch 0004** (the scratches become early-clobber). Before: the test exits 7; after: 0 | `t6_fpcr.c`; openamigartg `tools/fpcr_check.py` on built programs |
| **Loop shift becomes a forward copy.** `for (i = n; i > 0; --i) v[i] = v[i-1];` at `-O2` | the stove's 16.2.0b (sntrup761, 8 Oct) | Fixed in bebbo's tree: `569d72d67` "movmemsi must not guess the copy direction" (committed 7 Oct). Programs still build with `-fno-tree-loop-distribute-patterns` | `t10_memmove.c`, built without that flag |
| **`-m68060` 64-bit values read from the wrong stack slot** (openamigatls `build-amiga-tls.sh`, found 9 Oct) | both GCC 16 stoves, as recorded | **Not reproduced** in the compilers. Upstream 16.2.0 (reload and LRA) and bebbo amiga16.2 of 7 and 11 Oct, all at `-m68060`, give the PC's results for BearSSL as OpenTLS's CPU=060 build uses it, for OpenCrypto, for `t7_64bit.c` and for 222 of GCC's execute torture tests that use 64-bit values. Not covered: the stove's own `libm060` libraries (libgcc, libnix built by the stove) and the stove's exact GCC commit (`1ff531f6e`, not in the public mirror) | `t7`–`t9` built for the 060 on the real stove |
| `-m68060` with the FPU linked the 68000 libraries; `-mhard-float` linked soft-float libm | bebbo amiga16.2 before 9 Oct | Fixed in bebbo's tree: `50a8efdec` | build with `-m68881` and check the multilib (`-print-multi-directory`) |
| A read through a pointer GCC proves null becomes `TRAP #7` | all GCC 16 | Not a bug: address 0 is memory on an Amiga. Build with `-fno-delete-null-pointer-checks` | — |

**bebbo's current `amiga16.2` passes GCC's execute torture suite under qemu-m68k.** With patch 0004, at `-m68020 -m68881 -O2`, on 11 October:

| Result | Tests |
| --- | ---: |
| Pass | 1,496 |
| Fail | 14 |
| Couldn't build in the harness | 182 |

All 14 failures fail identically with upstream GCC there. They need `-fwrapv`, trampolines, `-finstrument-functions` or a full C library.

bebbo's tree has also fixed, since the start of October:
- `cse` and post-increment addresses (`gcc.c-torture/execute/va-arg-22.c` at `-O2`);
- overlapping auto-increment moves in HI, QI and SF modes (PR 123853);
- block move and set sizes and CPUs;
- `ext-dce` paradoxical DImode subregs without LRA;
- arguments bound to registers in their declared mode;
- sibling calls over `a0` arguments;
- `regparm (0)` on attributed function types;
- several `-fbaserel` cases.

The stove was built from an earlier commit. **Rebuild it from bebbo's current
`amiga16.2` with patches 0001–0004**, then run the tests here, CPU=060 builds
included, before lifting OpenTLS's refusal of GCC 16 for the 060.

## The tests

| Test | What | Built for |
| --- | --- | --- |
| t1 | C++ exceptions through a static library and through C built with `-fexceptions`; destructors on unwind; rethrow; `exception_ptr` | 020 |
| t2 | `dynamic_cast` and `typeid` across translation units, virtual inheritance, `bad_cast` | 020 |
| t3 | `std::thread`, `mutex`, `condition_variable`, `thread_local`, `call_once`, `async` | 020 |
| t4 | 64-bit `atomic`, `atomic_ref`, `compare_exchange`, `atomic<double>` | 020 |
| t5 | a 3,000-case `switch` (`-mlong-jump-table-offsets`) | 020 |
| t6 | the FPCR clash | 040 + 68881 |
| t7 | 64-bit arithmetic: stack arguments, spills, shifts, multiplies, divides, varargs, structs | 020, 060 |
| t8 | BearSSL as OpenTLS's CPU=060 build compiles it (i15, ctmul32, m15 curves), output equal to `expected/` | 020, 060 |
| t9 | OpenCrypto (SHA-2, X25519, Ed25519, ChaCha20-Poly1305, AES, RSA, ECDSA, sntrup761), output equal to `expected/` | 020, 060 |
| t10 | the loop shift and an overlapping backward copy | 020, 060 |

```sh
cd os32-gcc16/tests
STOVE=~/AmigaChrome/stoves/os32-gcc16 OPENTLS=~/openamigatls ./build-tests.sh
# copy bin/ and run-tests to the instance, Execute run-tests, copy results/ back
./check-results.sh results
```

`expected/` was made on a PC (x86-64, GCC 13) from the same sources.
