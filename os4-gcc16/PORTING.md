# How the AmigaOS 4 patches were ported (14 October 2026)

`patches/` holds our ports of AmigaLabs' adtools patches to GCC 16.2.0 and
binutils 2.47. This note records what had to change, so the next port (GCC 17,
binutils 2.48) starts from here.

Both patches are plain unified diffs against the pristine GNU release trees.
`patch -p1 -F0` applies each one with no fuzz. They include the regenerated
`configure` and `Makefile.in` files: autoconf 2.69, automake 1.15.1 and
`make headers` in bfd, the versions those trees were generated with. Each
regenerated file was checked to differ from pristine only by the intended
change.

## binutils 2.47 (from AmigaLabs `binutils/2.40`, `063184c1`)

46 files, +2133/-26. Its tests are as and ld on PowerPC objects, `ld -q`
keeping relocations, base-relative relocations against `_DATA_BASE_`, strip,
objcopy, ar, nm and readelf (66 checks, all passing).

- **Base-relative relocations.** `R_PPC_AMIGAOS_BREL`, `_LO`, `_HI` and `_HA`
  are types 210 to 213. Since 2.39, the size field of `HOW()` is a byte
  count, not log2, so the sizes are now 4/2/2/2. `_HI` shifts by 16, and
  `_HA` uses `ppc_elf_addr16_ha_reloc`. With the 2.40 values, 2.47's gas
  aborts and ld writes wrong bytes.
- **gas `@brel` in data directives.** These are left to the linker
  (`fx_done = 0`); 2.40's `md_apply_fix` wrote 2 bytes into a 4-byte field.
- **The AmigaOS ELF vector** (`powerpc_elf32_amigaos_vec`, `elf-amigaos.c`).
  It builds only under `OBJ_MAYBE_ELF_AMIGAOS`, and it restores the
  `ELF_TARGET_OS` and back-end defaults afterwards. In 2.40, the FreeBSD and
  VxWorks vectors that follow it inherited its overrides.
- **New in 2.47.** The `DT_AMIGAOS_DYNVERSION` tag moved to
  `ppc_elf_late_size_sections`, and bfd uses the 2.47 signatures
  (`info->output_bfd`, `finish_dynamic_symbol (info, h, sym)`). The NaCl
  backend is gone, `elf-amigaos.[ch]` are new files, and `is_amigaos` is the
  third ELF target OS.
- **Fixes for GCC 14 and later.** `index()` became `strchr()` with
  `string.h`, and libiberty's `pex-amigaos.c` takes the GCC 15 patch's correct
  prototypes.
- **Left out.** The gdb, gdbsupport, readline and gnulib parts serve a native
  AmigaOS gdb, and the binutils release has no gdb. The `.gitignore` lines and
  an accidental edit to `config.sub`'s licence text were also left out.

## GCC 16.2.0 (from AmigaLabs `gcc/15`, `87ac263c`)

83 files, +5159/-73. It builds as a cross compiler. Its 134 feature checks
pass: base-relative data, `linearvarargs`, interface (`libcall`) calls in C
and C++, `checktags`, the predefines, `-mcrt=` and the link line. All 2003
`gcc.c-torture/compile` files compile with no internal errors at `-O2`,
`-O0`/`-O2`/`-Os -mbaserel`.

Four of these were bugs in the GCC 15 patch, which the feature tests found:
- **`-mbaserel` crashed on every data access.** The `elf_base_high` and
  `elf_base_low` patterns sat below `elf_high` and `elf_low`, which match
  anything, so they never matched. They now come first and emit
  `@brel@ha`/`@brel@l`, as adtools did up to GCC 11. The GCC 15 patch emitted
  absolute `@ha`/`@l`. `rs6000_emit_move` loads base-relative data addresses
  through r2 again.
- **`va_getlinearva(v, t)`** is `((t)__builtin_getlinearva(v))` again. The
  GCC 15 form passes `&(v)`, which C and C++ reject since GCC 14.
- **`checktags`** missed tags declared `static const int`, because they are
  folded or wrapped in a conversion. It now looks at the unfolded argument.
- **An error in `__builtin_getlinearva`** returned a null rtx, which crashed
  expand. It now returns `const0_rtx`.

These hunks were adapted:
- **`baserel_restore` functions.** These are functions that reload r2 with
  `bl __baserel_get_addr`. They get no sibcalls and no separate
  shrink-wrapping (the call clobbers LR), and r2 is set up before its first
  use.
- **`#ifdef CALL_LINEARVARARGS`** guards in `rs6000-call.cc`, and
  `RS6000_BASEREL` in `rs6000.h`. With these, other rs6000 targets still
  compile.
- **C++ interface calls (`cp/typeck.cc`)** build their own argument vector.
  GCC 15's version released the caller's vector twice.
- **Hunks re-applied by hand** next to new GCC 16 code: `g++spec.cc`,
  `cp/parser.cc`, and the attribute documentation. The docs are now one node,
  "AmigaOS PowerPC Attributes".
- **`_Static_warning`** now warns. In the GCC 15 patch it failed like
  `_Static_assert`, although the code computed a "warning only" flag.
- **libstdc++'s `crossconfig.m4`.** Its AmigaOS branch now runs as written
  (the GCC 15 patch shipped a hand-edited `configure`). Its link tests run
  only when the C library can be linked.
- **`LT_OUTPUT`** in libstdc++'s `configure.ac` was left out. No AmigaOS
  build ever ran it.

Known issues carried over from the GCC 15 patch (not porting mistakes):
- Hyperion's SDK 54.25 has newlib start-up files for `-mbaserel` but no
  base-relative `libc.a`, so `-mbaserel` with newlib stops at `cannot find
  -lc`, as with adtools. clib2 builds a base-relative `libc.a`: use
  `-mcrt=clib2 -mbaserel`.
- The driver links no thread object by default, so a C++ program that uses
  libstdc++ needs `-athread=native` or `-athread=single`; without it the link
  stops at undefined `__gthread_*` symbols. adtools documents the same.
- `-mcrt=clib2-ts` (clib2's thread-safe build) has no multilib, so C++ with it
  links the newlib libstdc++.
- `-athread=pthread` names `gthr-amigaos-pthread.o`, which libgcc never builds.
- `-flto` on code that uses `va_getlinearva` fails at the link-time compile.
- The multilibs are newlib and clib2 only; `-mcrt=clib4` links the newlib
  libgcc and libstdc++.
- `check68kfuncptr` is accepted but generates no code.
- The parts for a compiler that runs on AmigaOS itself (`x-amigaos`,
  `config.host`, `PROGDIR` handling) are carried but untested.
