#!/usr/bin/env bash
# Build the aros68k-gcc16 stove: GCC 16.2.0 and binutils 2.47 for AROS 68k,
# through AROS's own crosstools build, from pinned and verified sources.
#
#   ./build.sh [--work DIR] [--aros-src DIR] [--jobs N] [--verify-only]
#
#   --work DIR      where everything goes (default: ./work)
#                     ports/      sources, checked against sources.lock
#                     sdk-build/  the AROS crosstools build; it also holds the
#                                 AROS headers and libraries the compiler
#                                 uses as its sysroot, so keep it
#                     toolchain/  the compiler: toolchain/m68k-aros-gcc
#   --aros-src DIR  an AROS checkout already at the pinned ref (saves a clone)
#   --jobs N        parallel make jobs (default 8)
#   --verify-only   fetch nothing; check the sources in ports/ and stop
#
# Host needs: git curl xz bzip2 make gcc g++ bison flex gawk perl python3
# texinfo autoconf automake, and gpgv to check the GNU signatures.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$HERE/work"; AROS_SRC=""; JOBS=8; VERIFY_ONLY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --work) WORK="$2"; shift 2 ;;
        --aros-src) AROS_SRC="$2"; shift 2 ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --verify-only) VERIFY_ONLY=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done
AROS_REPOSITORY=https://github.com/aros-development-team/AROS.git
AROS_REF=c8860e674d7c5fbab304e2bf201f7a9c81c98629
GCC_VERSION=16.2.0
BINUTILS_VERSION=2.47
PORTS="$WORK/ports"; SDK="$WORK/sdk-build"; PREFIX="$WORK/toolchain"; LOGS="$WORK/logs"
mkdir -p "$PORTS"

# 1. Sources: fetch what is missing, then check every file against sources.lock.
while read -r sha name url; do
    case "$sha" in ''|'#'*) continue ;; esac
    if [ ! -f "$PORTS/$name" ]; then
        [ "$VERIFY_ONLY" = 1 ] && { echo "missing: $name" >&2; exit 3; }
        echo "fetching $name"
        curl -fsSL -o "$PORTS/$name.part" "$url"
        mv "$PORTS/$name.part" "$PORTS/$name"
    fi
    echo "$sha  $PORTS/$name" | sha256sum -c --quiet - || { echo "checksum mismatch: $name" >&2; exit 4; }
done < "$HERE/sources.lock"
if command -v gpgv >/dev/null 2>&1; then
    for f in "gcc-$GCC_VERSION.tar.xz" "binutils-$BINUTILS_VERSION.tar.bz2"; do
        gpgv --keyring "$PORTS/gnu-keyring.gpg" "$PORTS/$f.sig" "$PORTS/$f" 2>&1 | grep -q "Good signature" \
            || { echo "bad GNU signature: $f" >&2; exit 4; }
    done
    echo "GNU signatures good"
else
    echo "gpgv not found: checksums checked, GNU signatures not" >&2
fi
# AROS fetches isl as .tar.bz2; isl publishes .tar.xz. Same tarball, repacked here.
[ -f "$PORTS/isl-0.27.tar.bz2" ] || xz -dc "$PORTS/isl-0.27.tar.xz" | bzip2 -9 > "$PORTS/isl-0.27.tar.bz2"
echo "sources verified"
[ "$VERIFY_ONLY" = 1 ] && exit 0

# 2. AROS at the pinned ref. Its tools/crosstools/gnu carries the AROS patches
#    for GCC 16.2.0 and binutils 2.47, m68k-aros included.
if [ -z "$AROS_SRC" ]; then
    AROS_SRC="$WORK/aros-src"
    [ -d "$AROS_SRC/.git" ] || git clone "$AROS_REPOSITORY" "$AROS_SRC"
    git -C "$AROS_SRC" checkout -q --detach "$AROS_REF"
    git -C "$AROS_SRC" submodule update --init --recursive -q
fi
got="$(git -C "$AROS_SRC" rev-parse HEAD)"
[ "$got" = "$AROS_REF" ] || { echo "AROS source is at $got, not $AROS_REF" >&2; exit 5; }

# 3. Configure and build the crosstools (about an hour on 8 cores).
mkdir -p "$SDK" "$PREFIX" "$LOGS"
echo "configuring (log: $LOGS/configure.log)"
( cd "$SDK" && "$AROS_SRC/configure" --target=amiga-m68k --with-serial-debug=yes \
    --with-gcc-version="$GCC_VERSION" --with-binutils-version="$BINUTILS_VERSION" \
    --with-portssources="$PORTS" --with-aros-toolchain-install="$PREFIX" ) > "$LOGS/configure.log" 2>&1
echo "building (log: $LOGS/build.log)"
( cd "$SDK" && make -j"$JOBS" crosstools ) > "$LOGS/build.log" 2>&1

# 3b. Defaults: address 0 is memory on a 68k Amiga (chip RAM, exec's pointer at 4).
# GCC assumes nothing lives there; at -O2 it turns a read through a pointer it
# has proved null into TRAP #7 and drops null checks after a read. A specs file
# beside libgcc makes -fno-delete-null-pointer-checks the default; a build can
# still ask for -fdelete-null-pointer-checks.
SPECS="$(dirname "$("$PREFIX/m68k-aros-gcc" -print-libgcc-file-name)")/specs"
printf '*cc1:\n+ -fno-delete-null-pointer-checks\n\n*cc1plus:\n+ -fno-delete-null-pointer-checks\n\n' > "$SPECS"
printf 'volatile unsigned long s;\nint main(void) { s = *(volatile unsigned long *)0; return 0; }\n' > "$LOGS/nullread.c"
"$PREFIX/m68k-aros-gcc" -O2 -c -o "$LOGS/nullread.o" "$LOGS/nullread.c"
if "$PREFIX/m68k-aros-objdump" -d "$LOGS/nullread.o" | grep -qE 'trap +#7'; then
  echo "the stove still turns a read of address 0 into TRAP #7 ($SPECS)"; exit 1
fi
echo "defaults: $SPECS (-fno-delete-null-pointer-checks)"

# 4. Identity: what this stove is, so it is never mistaken for another compiler.
python3 - "$WORK" "$AROS_REF" "$GCC_VERSION" "$BINUTILS_VERSION" <<'PY'
import json, sys, time, subprocess
from pathlib import Path
work, ref, gcc, binutils = Path(sys.argv[1]), *sys.argv[2:]
cc = work / "toolchain" / "m68k-aros-gcc"
identity = {
    "schema": 1, "id": "aros68k-gcc16", "name": "AROS 68k, GCC 16.2",
    "target": "m68k-aros", "gcc": gcc, "binutils": binutils, "arosRef": ref,
    "compilerVersion": subprocess.run([str(cc), "--version"], capture_output=True, text=True).stdout.splitlines()[0],
    "prefix": "toolchain", "sysroot": "sdk-build/bin/amiga-m68k/AROS/Developer",
    "defaultFlags": ["-fno-delete-null-pointer-checks"],
    "builtAt": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
}
(work / "stove.json").write_text(json.dumps(identity, indent=2) + "\n")
print(identity["compilerVersion"])
PY
echo "done: $PREFIX/m68k-aros-gcc"
