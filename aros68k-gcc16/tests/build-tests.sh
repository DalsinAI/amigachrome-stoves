#!/bin/sh
# Build every test with GCC 16.2 (this stove) and, when given, GCC 6.5, side by side:
#   bin/<test>-g16   from G16 (default: ../work/toolchain/m68k-aros-)
#   bin/<test>-g6    from G6, if set (AROS's default compiler, for comparison)
# build.log keeps every warning. Copy bin/ and run-tests to DH1:GCC16/ on an
# AROS 68k machine and Execute run-tests; it writes DH1:GCC16/results.txt.
#
# C++ programs link with -Wl,-u,__pthread_Init_Func -lpthread -latomic. On AROS
# 68k libstdc++ takes its locks through pthread; when only libstdc++ pulls the
# pthread objects in, collect-aros leaves pthread's ADD2INIT entry out of the
# init set, its semaphores stay zeroed and the first lock waits for ever (both
# GCC 6.5 and 16.2). Forcing __pthread_Init_Func in early puts it back.
CXXLINK="-Wl,-u,__pthread_Init_Func -lpthread -latomic"
cd "$(dirname "$0")"
G16="${G16:-$(cd .. && pwd)/work/toolchain/m68k-aros-}"
mkdir -p bin
: > build.log
build() {   # tag prefix test compiler std extra...
    tag=$1 pre=$2 t=$3 cc=$4 std=$5; shift 5
    src=$(ls src/$t*.c* | head -1)
    out=bin/$t-$tag
    echo "== $out ($("${pre}gcc" -dumpversion), $std $*)" >> build.log
    if "${pre}$cc" -O2 $std -Wall -o "$out" "$src" "$@" >> build.log 2>&1; then
        printf '%-12s %8s bytes\n' "$out" "$(wc -c < "$out")" | tee -a build.log
    else
        echo "$out: BUILD FAILED (see build.log)" | tee -a build.log
    fi
}
for v in g6 g16; do
    if [ $v = g6 ]; then [ -n "${G6:-}" ] || continue; pre=$G6 cxx=-std=gnu++14 c=-std=gnu11
    else pre=$G16 cxx=-std=gnu++20 c=-std=gnu23; fi
    build $v "$pre" t1 gcc "$c"
    build $v "$pre" t2 gcc "$c" -lm
    build $v "$pre" t3 g++ "$cxx" $CXXLINK
    build $v "$pre" t4 g++ "$cxx" $CXXLINK
    build $v "$pre" t5 g++ "$cxx" $CXXLINK
    build $v "$pre" t6 gcc "$c"
    build $v "$pre" t7 gcc "$c"
done
