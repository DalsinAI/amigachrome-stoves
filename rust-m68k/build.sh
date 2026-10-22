#!/usr/bin/env bash
# Build the rust-m68k stove: Rust 1.97.0 for the 68040 (m68k-unknown-none-elf),
# on LLVM 22.1 with our M68k backend fixes, from pinned sources.
#
#   ./build.sh [--work DIR] [--jobs N] [--llvm-only] [--no-link]
#
#   --work DIR    where everything goes (default: ./work)
#                   llvm-project/  rust-lang's LLVM at the pinned commit,
#                                  with patches/llvm applied
#                   llvm-build/    LLVM (keep it: rustc links to it)
#                   rust/          Rust at the pinned tag
#   --jobs N      parallel jobs (default: all cores)
#   --llvm-only   stop after LLVM (enough for tests/run-tests --llc)
#   --no-link     do not register the toolchain with rustup
#
# The toolchain is work/rust/build/x86_64-unknown-linux-gnu/stage1, linked as
# the rustup toolchain "rust-m68k". Programs are built with
#   cargo +rust-m68k build --target m68k-unknown-none-elf -Zbuild-std=core,compiler_builtins
# (see README.md for the flags).
#
# Host needs: git cmake ninja gcc g++ python3 curl, and rustup unless --no-link.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$HERE/work"; JOBS="$(nproc)"; LLVM_ONLY=0; LINK=1
while [ $# -gt 0 ]; do
    case "$1" in
        --work) WORK="$2"; shift 2 ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --llvm-only) LLVM_ONLY=1; shift ;;
        --no-link) LINK=0; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done
LLVM_REPOSITORY=https://github.com/rust-lang/llvm-project.git
LLVM_BRANCH=rustc/22.1-2026-05-19
LLVM_REF=08c84e69a84d95936296dfcab0e38b34100725d5
RUST_REPOSITORY=https://github.com/rust-lang/rust.git
RUST_TAG=1.97.0
RUST_REF=2d8144b7880597b6e6d3dfd63a9a9efae3f533d3
mkdir -p "$WORK"; WORK="$(cd "$WORK" && pwd)"
SRC="$WORK/llvm-project"; BUILD="$WORK/llvm-build"; RUST="$WORK/rust"

# 1. LLVM at the pinned commit (the one Rust 1.97.0 ships), with our patches.
if [ ! -d "$SRC/.git" ]; then
    git clone --filter=blob:none --single-branch -b "$LLVM_BRANCH" "$LLVM_REPOSITORY" "$SRC"
fi
git -C "$SRC" am --abort 2>/dev/null || true
git -C "$SRC" -c advice.detachedHead=false checkout -q --detach "$LLVM_REF"
got="$(git -C "$SRC" rev-parse HEAD)"
[ "$got" = "$LLVM_REF" ] || { echo "LLVM source is at $got, not $LLVM_REF" >&2; exit 5; }
git -C "$SRC" -c user.name=stove -c user.email=stove@localhost am -q --keep-cr "$HERE"/patches/llvm/*.patch
echo "LLVM $LLVM_REF with $(ls "$HERE"/patches/llvm/*.patch | wc -l) patches"

cmake -S "$SRC/llvm" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_TARGETS_TO_BUILD=X86 \
    -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=M68k -DLLVM_ENABLE_PROJECTS="" \
    -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF >/dev/null
ninja -C "$BUILD" -j "$JOBS"
ninja -C "$BUILD" -j "$JOBS" check-llvm-codegen-m68k check-llvm-mc-m68k check-llvm-mc-disassembler-m68k
[ "$LLVM_ONLY" = 1 ] && { echo "LLVM done: $BUILD/bin/llc"; exit 0; }

# 2. Rust at the pinned tag, built against that LLVM.
if [ ! -d "$RUST/.git" ]; then
    git clone --depth 1 -b "$RUST_TAG" "$RUST_REPOSITORY" "$RUST"
fi
got="$(git -C "$RUST" rev-parse HEAD)"
[ "$got" = "$RUST_REF" ] || { echo "Rust source is at $got, not $RUST_REF" >&2; exit 5; }
cat > "$RUST/bootstrap.toml" <<TOML
change-id = "ignore"
profile = "dist"
[llvm]
download-ci-llvm = false
[build]
host = ["x86_64-unknown-linux-gnu"]
target = ["x86_64-unknown-linux-gnu"]
docs = false
extended = false
tools = []
jobs = $JOBS
[rust]
channel = "stable"
download-rustc = false
debuginfo-level = 0
lld = false
llvm-tools = false
[target.x86_64-unknown-linux-gnu]
llvm-config = "$BUILD/bin/llvm-config"
TOML
( cd "$RUST" && python3 x.py build --stage 1 library )
STAGE1="$RUST/build/x86_64-unknown-linux-gnu/stage1"
"$STAGE1/bin/rustc" -vV
[ "$LINK" = 1 ] && rustup toolchain link rust-m68k "$STAGE1"
echo "rust-m68k ready: $STAGE1"
