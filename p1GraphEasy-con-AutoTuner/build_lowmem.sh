#!/usr/bin/env bash
# Low-memory rebuild of p1-AT's GraphProgram (one TU at a time, no -g).
# The prebuilt GraphProgram_new/_orig link against libantlr4-runtime 4.13.2,
# which is not installed here (system has 4.13.1) -- hence rebuilding.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"; cd "$ROOT"
LLVM_CONFIG=/usr/local/llvm-20-polly-rtti/bin/llvm-config
OBJ_DIR="${OBJ_DIR:-$ROOT/build_lowmem}"; mkdir -p "$OBJ_DIR"
LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"; LLVM_CXXFLAGS="${LLVM_CXXFLAGS//-fno-exceptions/}"
LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"; LLVM_LIBS="$($LLVM_CONFIG --libs all)"
LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"
# -iquote, not -I., so #include <random> cannot hit the stray ELF named `random`
CXXFLAGS=(-O2 -mavx2 -march=native -std=c++17 -fexceptions -pthread
          -I/usr/local/include/antlr4-runtime -Igenerated -iquote . $LLVM_CXXFLAGS)
SOURCES=(main.cpp IRGenVisitor.cpp MotifPattern.cpp MotifIRBuilder.cpp ASTBuilder.cpp pdg.cpp parallel_loop_outline.cpp
         SemanticAnalyzer.cpp roaring_bitmap.cpp AutoTunerPass.cpp
         generated/BaseBaseListener.cpp generated/BaseBaseVisitor.cpp
         generated/BaseLexer.cpp generated/BaseListener.cpp
         generated/BaseParser.cpp generated/BaseVisitor.cpp)
OBJECTS=()
echo "=== compiling ${#SOURCES[@]} sources ==="
for src in "${SOURCES[@]}"; do
  obj="$OBJ_DIR/$(echo "$src" | tr '/' '_' | sed 's/\.cpp$/.o/')"; OBJECTS+=("$obj")
  # Rebuild when the source OR any local header is newer.  Comparing against
  # the .cpp alone let a changed SemanticAnalyzer.h / IRGenVisitor.h leave other
  # objects compiled against the previous class layout, which corrupts memory at
  # run time instead of failing to link.
  stale=0
  [[ -f "$obj" ]] || stale=1
  if [[ "$stale" == 0 ]]; then
    for hdr in *.h; do [[ "$hdr" -nt "$obj" ]] && { stale=1; break; }; done
  fi
  [[ "$src" -nt "$obj" ]] && stale=1
  if [[ "$stale" == 0 ]]; then echo "  skip: $src"; continue; fi
  echo "  cc: $src"; g++ -c "${CXXFLAGS[@]}" "$src" -o "$obj"
done
echo "=== linking GraphProgram ==="
g++ "${OBJECTS[@]}" $LLVM_LDFLAGS -Wl,--no-keep-memory -Wl,--reduce-memory-overheads \
  -pthread -lantlr4-runtime -lPolly -lPollyISL -lisl $LLVM_LIBS $LLVM_SYSTEM_LIBS -o GraphProgram
echo ">>> p1-AT GraphProgram build complete"

# Dense semiring-closure runtime (autograph_closure) — link into user programs.
if [[ -f semiring_runtime.c ]]; then
  echo "=== compiling semiring_runtime.o ==="
  gcc -O3 -fopenmp -iquote . -c semiring_runtime.c -o semiring_runtime.o
fi
echo "Link user programs with: parallel_runtime_shim.o semiring_runtime.o (and the usual loaders)"
