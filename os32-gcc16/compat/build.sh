#!/bin/sh
# Copyright (c) 2026 AmigaChrome Team. MIT licence.
# libos32compat.a for the os32-gcc16 stove: the symbols libstdc++ (GCC 16) expects
# and libnix lacks, aligned allocation, and long double maths through double.
#   build.sh [OUT_DIR]       (default: this folder)   STOVE (default ~/AmigaChrome/stoves/os32-gcc16)
# Link it after libstdc++: -lstdc++ -L<stove>/compat -los32compat, with
# -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free for aligned_alloc.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
STOVE=${STOVE:-$HOME/AmigaChrome/stoves/os32-gcc16}
OUT=${1:-$HERE}
P=$STOVE/prefix/bin/m68k-amigaos-
# These reproduce the installed objects exactly: the compat sources at -O2,
# the long double wrappers at -Os -fno-builtin (so each calls the double function).
COMPAT="-m68020 -m68881 -O2 -Wall"
LDBL="-m68020 -m68881 -Os -fno-builtin -Wall"
# One library per code model: OUT/libos32compat.a (absolute), OUT/libb/ (-fbaserel) and
# OUT/libb32/ (-fbaserel32). Base-relative programs need their own: absolute-model code
# uses a4 as a scratch register, and the malloc wrappers call back into libnix.
build() {   # build SUBDIR EXTRA_FLAGS
    o_dir="$OUT/obj$1"; mkdir -p "$o_dir" "$OUT$1"; objs=
    for s in "$HERE"/os32compat.c "$HERE"/os32compat_malloc.c "$HERE"/os32compat_math.c; do
        o="$o_dir/$(basename "${s%.c}").o"; "${P}gcc" $COMPAT $2 -c "$s" -o "$o"; objs="$objs $o"
    done
    for s in "$HERE"/ldbl/*.c; do
        o="$o_dir/$(basename "${s%.c}").o"; "${P}gcc" $LDBL $2 -c "$s" -o "$o"; objs="$objs $o"
    done
    for s in "$HERE"/os32compat_cxx.cpp "$HERE"/os32compat_statvfs.cpp; do
        o="$o_dir/$(basename "${s%.cpp}").o"; "${P}g++" $COMPAT $2 -c "$s" -o "$o"; objs="$objs $o"
    done
    rm -f "$OUT$1/libos32compat.a"
    # shellcheck disable=SC2086
    "${P}ar" rcs "$OUT$1/libos32compat.a" $objs
    echo "$OUT$1/libos32compat.a ($(wc -c < "$OUT$1/libos32compat.a") bytes, $(echo $objs | wc -w) objects)"
}
build "" ""
build /libb -fbaserel
build /libb32 -fbaserel32
