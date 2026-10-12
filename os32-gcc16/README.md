# os32-gcc16: tests

The os32-gcc16 stove is built by [DalsinAI/openamigagcc](https://github.com/DalsinAI/openamigagcc):
bebbo's amiga-gcc with GCC branch `amiga16.2` (GCC 16.2.0b) and that
repository's patches. The stove on the Team's PC was rebuilt on 9 October 2026
with patches 0001-0009, which fix among others the 68040 FPCR clash (0003),
the memmove direction (0004) and the 64-bit libcall argument pushed from a stack
slot that broke `-m68060`/`-m68040` builds (0008). openamigagcc's
`tests/repro/prove.sh` proves each of those under qemu-m68k.

This folder adds broader qualification tests, which need no NDK:

- `tests/`: nineteen programs (and t3's pieces, t3a-t3h) built with `tests/build-tests.sh`, run on an instance
  with `tests/run-tests` and judged with `tests/check-results.sh`. MIT.
- `tests/qemu-check/`: how upstream GCC 16.2.0 and bebbo's public `amiga16.2`
  were checked on 11 October 2026 without an Amiga, under qemu-m68k. MIT.

## What the 11 October checks showed

- **openamigagcc's fixes are still needed.** Its push64 reproducer is still
  wrong with upstream GCC 16.2.0 and with bebbo's public `amiga16.2` (the
  AmigaPorts mirror, 7 and 11 October) at `-m68060`, and the FPCR clash still
  shows there with LRA. Both fixes are worth offering upstream.
- **With openamigagcc's FPCR change applied, bebbo's public `amiga16.2` passes
  GCC's execute torture suite**, at `-m68020 -m68881 -O2`:

  | Result | Tests |
  | --- | ---: |
  | Pass | 1,496 |
  | Fail | 14 |
  | Couldn't build in the harness | 182 |

  All 14 failures fail identically with upstream GCC there. They need
  `-fwrapv`, trampolines, `-finstrument-functions` or a full C library.
- **BearSSL and OpenCrypto agree with a PC at `-m68060`.** Built the way
  OpenTLS's CPU=060 build compiles them, both give the PC's output with every
  compiler tried. That code doesn't contain the push64 pattern, which is why
  t8 and t9 alone could not show it.

## The tests

| Test | What | Built for |
| --- | --- | --- |
| t1 | C++ exceptions through a static library and through C built with `-fexceptions`; destructors on unwind; rethrow; `exception_ptr` | 020 |
| t2 | `dynamic_cast` and `typeid` across translation units, virtual inheritance, `bad_cast` | 020 |
| t3 | `std::thread`, `mutex`, `condition_variable`, `thread_local`, `call_once`, `async` | 020 |
| t4 | 64-bit `atomic`, `atomic_ref`, `compare_exchange`, `atomic<double>` | 020 |
| t5 | a 3,000-case `switch` (`-mlong-jump-table-offsets`) | 020 |
| t6 | the FPCR clash (as openamigagcc 0003 fixed it) | 040 + 68881 |
| t7 | 64-bit arithmetic: stack arguments, spills, shifts, multiplies, divides, varargs, structs | 020, 060 |
| t8 | BearSSL as OpenTLS's CPU=060 build compiles it (i15, ctmul32, m15 curves), output equal to `expected/` | 020, 060 |
| t9 | OpenCrypto (SHA-2, X25519, Ed25519, ChaCha20-Poly1305, AES, RSA, ECDSA, sntrup761), output equal to `expected/` | 020, 060 |
| t10 | the loop shift and an overlapping backward copy | 020, 060 |
| t11 | exceptions thrown and caught in 8 threads; carried out of `async`, `promise`, `packaged_task` and an `exception_ptr` | 020 |
| t12 | `thread_local` objects with constructors and destructors in 80 threads: destroyed by the time `join()` returns, counts equal | 020 |
| t13 | 50, then 100 threads alive at once (past libpthread's 22- and 94-slot table sizes), each with `thread_local` data | 020 |
| t14 | `shared_mutex` readers and writers; `shared_timed_mutex` `try_lock_for` and `try_lock_shared` | 020 |
| t15 | `jthread`, `stop_token`, `stop_callback`, `condition_variable_any::wait` with a `stop_token` | 020 |
| t16 | `atomic` wait/notify, `atomic_flag` wait, `latch`, `barrier`, `counting_semaphore` | 020 |
| t17 | `condition_variable_any` with `mutex`, `shared_mutex` and a lock of its own; `wait_for` timing out | 020 |
| t18 | `ostringstream`/`istringstream` (ints, hex, doubles) in 8 threads; `cout` from several threads | 020 |
| t19 | 1,150 short threads: one after another, in batches, detached, `async`; `thread_local` destructors | 020 |
| t12c, t13c, t19c | t12, t13 and t19 built with `-fuse-cxa-atexit` (and `src/cxa_dso.c` for `__dso_handle`), so that `thread_local` destructors are registered | 020 |

## What the tests show on AmigaOS (lab of Instance-32, 11 October)

With the stove plus openamigagcc PRs #6 (libpthread) and #7 (libnix FPCR),
t1-t11, t14-t17, t3 (20 runs) and t12c/t13c pass, and t7-t9 equal `expected/`
for the 020 and the 060. The live stove fails t3 and t7-060. Still failing
with both fixes, each a problem of its own:

- **t12, t13, t19: no `thread_local` destructor runs.** The stove defaults to
  `-fno-use-cxa-atexit`, and GCC then emits no `__cxa_thread_atexit` call for a
  `thread_local`. Built with `-fuse-cxa-atexit` (t12c, t13c) they pass.
- **t19: `std::thread::detach()` throws** `system_error` ("No such process"):
  libpthread's `pthread_detach` is not implemented and returns ESRCH.
- **t18 ends the Shell running it** soon after 8 threads start formatting
  doubles with `ostringstream`. Still open.

```sh
cd os32-gcc16/tests
STOVE=~/AmigaChrome/stoves/os32-gcc16 OPENTLS=~/openamigatls ./build-tests.sh
# copy bin/ and run-tests to the instance, Execute run-tests, copy results/ back
./check-results.sh results
```

`expected/` was made on a PC (x86-64, GCC 13) from the same sources.
