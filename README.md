# AmigaChrome stoves

Stoves are the compilers AmigaChrome's Kitchen cooks Amiga builds with. This
repository holds the recipe for each one: pinned and verified sources, a
build script, an identity file, and the tests that prove it works on the
machine it targets. Built stoves are published as releases.

| Stove | Compiler | Builds programs for | Status |
| --- | --- | --- | --- |
| [aros68k-gcc16](aros68k-gcc16/) | GCC 16.2.0, binutils 2.47 | AROS 68k | Experimental |
| [os32-gcc16](os32-gcc16/) | bebbo's GCC 16.2.0b (`amiga16.2`) | AmigaOS 3.x | Tests only: built by [openamigagcc](https://github.com/DalsinAI/openamigagcc) |

Each stove says what it is in `stove.json`, so it is never mistaken for
another compiler: a stove is added beside the ones that work, never in place
of them.

The AmigaOS 3.2 stoves are not built here. They need the AmigaOS NDK, which is
not ours to redistribute; they stay on the machines that hold their own NDK.
`os32-gcc16/` holds tests for the GCC 16 one; its recipe and patches are DalsinAI/openamigagcc.

## Licences

The scripts, tests and documents here are MIT licensed (see LICENSE).

The compilers are built from third-party sources under their own licences:
GCC and binutils under the GPL (version 3 or later), with GCC's runtime
library exception; gmp, mpfr and mpc under the LGPL; isl under the MIT
licence; AROS and its headers and libraries under the AROS Public License.
Every release that carries binaries also carries the exact sources and
patches they were built from. Programs you compile with a stove can be under
any licence you choose.

## Contributors

AmigaChrome stoves is created and maintained by [SacredTrees](https://github.com/SacredTrees) with the AmigaChrome agent team, copyright Dalsin Limited. Everyone whose work it includes is credited in [`CONTRIBUTORS.md`](CONTRIBUTORS.md).
