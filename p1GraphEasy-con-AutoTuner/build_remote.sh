#!/usr/bin/env bash
#
# Building p1-AT against a *distribution* LLVM (used on the CUDA box).
#
# Prerequisites on a fresh machine (Ubuntu 24.04 shown):
#   * an LLVM 20 with the NVPTX backend and Polly:
#       wget -qO /tmp/llvm.sh https://apt.llvm.org/llvm.sh && chmod +x /tmp/llvm.sh && /tmp/llvm.sh 20
#       apt-get install -y llvm-20-dev libpolly-20-dev libisl-dev \
#                          libnlopt-dev libisl-dev time
#     (`time` matters: the suite's scaling check times runs with
#      /usr/bin/time -f 'T=%e'; a minimal image without it makes that check
#      report the 99999 sentinel instead of a real speedup.)
#   * the ANTLR C++ runtime matching the committed generated parser (4.13.1,
#     not the 4.10 in the distro) installed under /usr/local with
#     /usr/local/include/antlr4-runtime and /usr/local/lib/libantlr4-runtime.so
#   * ln -sfn /usr/lib/llvm-20 /usr/local/llvm-20-polly-rtti (the path the
#     repo's other build scripts expect)
#
# Differences from build_lowmem.sh: parallel TU compilation, -L/usr/local/lib so
# the shipped ANTLR wins, and the Polly archives only when LLVM does not already
# provide Polly (see the comment at the link step).
# ANTLR 4.13.1 runtime.  Mirrors build_lowmem.sh, but
#   * -L/usr/local/lib so the shipped ANTLR 4.13.1 wins over distro 4.10,
#   * TUs are compiled in parallel (this box has 48 cores),
#   * Polly archives are only added when LLVM does not already provide Polly
#     (distro llvm-config --libs all is a monolithic -lLLVM-20, and linking the
#     static Polly on top of that registers every cl::opt twice, which makes the
#     compiler abort at startup with "registered more than once").
# A failed compile must not be mistaken for a successful one.
rm -f GraphProgram
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"; cd "$ROOT"
LLVM_CONFIG=/usr/local/llvm-20-polly-rtti/bin/llvm-config
OBJ_DIR="${OBJ_DIR:-$ROOT/build_remote}"; mkdir -p "$OBJ_DIR"
LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"; LLVM_CXXFLAGS="${LLVM_CXXFLAGS//-fno-exceptions/}"
LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"; LLVM_LIBS="$($LLVM_CONFIG --libs all)"
LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"
CXXFLAGS=(-O2 -mavx2 -march=native -std=c++17 -fexceptions -pthread
          -I/usr/local/include/antlr4-runtime -Igenerated -iquote .
          -I"$($LLVM_CONFIG --includedir)/polly" $LLVM_CXXFLAGS)
SOURCES=(main.cpp IRGenVisitor.cpp MotifPattern.cpp MotifIRBuilder.cpp ASTBuilder.cpp pdg.cpp
         parallel_loop_outline.cpp graph_frontier_lowering.cpp SemanticAnalyzer.cpp roaring_bitmap.cpp
         AutoTunerPass.cpp generated/BaseBaseListener.cpp generated/BaseBaseVisitor.cpp
         generated/BaseLexer.cpp generated/BaseListener.cpp generated/BaseParser.cpp generated/BaseVisitor.cpp)
OBJS=()
: > "$OBJ_DIR/compile_list.txt"
for src in "${SOURCES[@]}"; do
  obj="$OBJ_DIR/$(echo "$src" | tr "/" "_" | sed "s/\.cpp$/.o/")"
  OBJS+=("$obj")
  stale=0
  [[ -f "$obj" ]] || stale=1
  if [[ "$stale" == 0 ]]; then
    for hdr in *.h; do [[ "$hdr" -nt "$obj" ]] && { stale=1; break; }; done
  fi
  [[ "$src" -nt "$obj" ]] && stale=1
  if [[ "$stale" == 0 ]]; then echo "  skip: $src"; continue; fi
  printf "%q " g++ -c "${CXXFLAGS[@]}" "$src" -o "$obj" >> "$OBJ_DIR/compile_list.txt"
  printf "\n" >> "$OBJ_DIR/compile_list.txt"
done
echo "=== compiling $(wc -l < "$OBJ_DIR/compile_list.txt") TU(s), ${JOBS:-8} jobs ==="
xargs -P "${JOBS:-8}" -I CMD bash -c CMD < "$OBJ_DIR/compile_list.txt"
POLLY_EXTRA=()
if ! grep -q -- "-lLLVM" <<< "$LLVM_LIBS"; then
  POLLY_EXTRA=(-lPolly -lPollyISL -lisl)
fi
echo "=== linking GraphProgram (polly archives: ${#POLLY_EXTRA[@]}) ==="
g++ "${OBJS[@]}" $LLVM_LDFLAGS -Wl,--no-keep-memory -Wl,--reduce-memory-overheads \
  -pthread -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lantlr4-runtime \
  "${POLLY_EXTRA[@]}" $LLVM_LIBS $LLVM_SYSTEM_LIBS -o GraphProgram
if [ ! -f GraphProgram ]; then echo ">>> GraphProgram MISSING: build failed"; exit 1; fi
echo ">>> remote GraphProgram build complete"
