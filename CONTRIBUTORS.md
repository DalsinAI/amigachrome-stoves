# Contributors

## Creator and maintainer

- **SacredTrees** ([@SacredTrees](https://github.com/SacredTrees)): created and maintains these AmigaChrome stove builds. The compilers themselves are the work of their upstream projects, credited below.

## The AmigaChrome team

We are the AI agents who build the AmigaChrome stoves alongside SacredTrees:

- **Agnus**, our coordinator, who keeps every thread moving.
- **Thufir**, **Kynes** and **Galen**, the earlier agents who started the work on SacredTrees's x86 cores.
- **The Claude Code threads**, each one taking a piece of the work from design to release.

## Copyright holder

The build scripts, `sources.lock` files, tests and documents are
Copyright (c) 2026 AmigaChrome Team, released under the MIT licence (`LICENSE`).

## Fetched at build time, not committed

`aros68k-gcc16/build.sh` and `os4-gcc16/build.sh` download these, each checked
against the SHA-256 in their `sources.lock` (GCC and binutils also against
their GNU signatures), or at a pinned git commit. Every release that carries binaries also carries the exact
sources and patches they were built from.

| Component | Pinned | Authors | Licence |
| --- | --- | --- | --- |
| GCC | 16.2.0 | The GCC developers, Free Software Foundation | GPL 3 or later, with the GCC Runtime Library Exception |
| GNU binutils | 2.47 | The binutils developers, Free Software Foundation | GPL 3 or later |
| GMP | 6.3.0 | The GMP developers | LGPL |
| MPFR | 4.2.2 | The MPFR developers | LGPL |
| GNU MPC | 1.4.1 | The MPC developers | LGPL |
| isl | 0.27 | The isl developers | MIT |
| AROS, with its crosstools patches for m68k-aros and its headers and libraries | commit `c8860e674d` | The AROS Development Team | AROS Public License |
| clib2, a C library for AmigaOS 4 (os4-gcc16) | sodero/clib2 commit `1a42c693` | Olaf Barthel and the clib2 contributors | BSD 3-clause, with parts from musl (MIT) |
| clib4, a C library for AmigaOS 4 (os4-gcc16) | AmigaLabs/clib4 commit `30e487d2` | The clib4 developers | BSD 3-clause, with some LGPL and GPL files and OpenLibm |

## Our patches build on

`os4-gcc16/patches/` are our ports of the AmigaOS 4 support in the adtools
project to GCC 16.2.0 and binutils 2.47. That support is the work of Sebastian Bauer, Ola Söder,
Josef Wegner and the other [adtools](https://github.com/sba1/adtools)
contributors, carried on to GCC 15 and binutils 2.40 by Georgios Sokianos and
[AmigaLabs](https://github.com/AmigaLabs/adtools). The
patches are GPL, like GCC and binutils.

The AmigaOS NDK is not included and not fetched: it is not ours to
redistribute. Hyperion's AmigaOS 4.1 SDK is not included either: os4-gcc16
downloads it on the machine that builds the stove, and it never leaves that
machine.

Amiga, AmigaOS and other product names are trademarks of their respective
owners.
