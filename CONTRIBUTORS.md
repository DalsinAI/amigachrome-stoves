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
Copyright (c) 2026 Dalsin Limited, released under the MIT licence (`LICENSE`).

## Fetched at build time, not committed

`aros68k-gcc16/build.sh` downloads these, each checked against the SHA-256 in
`aros68k-gcc16/sources.lock` (GCC and binutils also against their GNU
signatures). Every release that carries binaries also carries the exact
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

The AmigaOS NDK is not included and not fetched: it is not ours to
redistribute.

Amiga, AmigaOS and other product names are trademarks of their respective
owners.
