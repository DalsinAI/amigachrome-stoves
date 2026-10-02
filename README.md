# AmigaChrome stoves

Stoves are the compilers AmigaChrome's Kitchen cooks Amiga builds with. This
repository holds the recipe for each one: pinned and verified sources, a
build script, an identity file, and the tests that prove it works on the
machine it targets. Built stoves are published as releases.

| Stove | Compiler | Builds programs for | Status |
| --- | --- | --- | --- |
| [aros68k-gcc16](aros68k-gcc16/) | GCC 16.2.0, binutils 2.47 | AROS 68k | Experimental |

Each stove says what it is in `stove.json`, so it is never mistaken for
another compiler: a stove is added beside the ones that work, never in place
of them.

The AmigaOS 3.2 stove is not here. It needs the AmigaOS NDK, which is not ours
to redistribute; it stays on the machines that hold their own NDK.

## Licences

The scripts, tests and documents here are MIT licensed (see LICENSE).

The compilers are built from third-party sources under their own licences:
GCC and binutils under the GPL (version 3 or later), with GCC's runtime
library exception; gmp, mpfr and mpc under the LGPL; isl under the MIT
licence; AROS and its headers and libraries under the AROS Public License.
Every release that carries binaries also carries the exact sources and
patches they were built from. Programs you compile with a stove can be under
any licence you choose.
