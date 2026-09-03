#!/usr/bin/env bash
# Low-memory rebuild of GraphProgram: one translation unit at a time, no debug
# info, and a link tuned to trade speed for RAM. Same flags as 1run.sh step [2]
# otherwise. Use when there is not enough free memory for the single-command
# -O3 -g build, which peaks at several GB while statically linking all of LLVM.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

LLVM_CONFIG=/usr/local/llvm-20-polly-rtti/bin/llvm-config
OBJ_DIR="${OBJ_DIR:-$ROOT/build_lowmem}"
mkdir -p "$OBJ_DIR"

LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"
LLVM_CXXFLAGS="${LLVM_CXXFLAGS//-fno-exceptions/}"
LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
LLVM_LIBS="$($LLVM_CONFIG --libs all)"
LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

# -O2 without -g: debug info is what makes the static LLVM link enormous.
# -mavx2 -march=native are required by the AVX2 intrinsics in roaring_bitmap.cpp.
CXXFLAGS=(-O2 -mavx2 -march=native -std=c++17 -fopenmp -fexceptions -pthread
          -I/usr/local/include/antlr4-runtime -Igenerated -I. $LLVM_CXXFLAGS)

SOURCES=(main.cpp IRGenVisitor.cpp MotifPattern.cpp MotifIRBuilder.cpp ASTBuilder.cpp SemanticAnalyzer.cpp roaring_bitmap.cpp
         generated/BaseBaseListener.cpp generated/BaseBaseVisitor.cpp generated/BaseLexer.cpp
         generated/BaseListener.cpp generated/BaseParser.cpp generated/BaseVisitor.cpp)

echo "=== [1] runtime.c ==="
gcc -c -O2 runtime.c -o "$OBJ_DIR/runtime.o"

echo "=== [2] compiling ${#SOURCES[@]} sources one at a time ==="
OBJECTS=("$OBJ_DIR/runtime.o")
for src in "${SOURCES[@]}"; do
  obj="$OBJ_DIR/$(echo "$src" | tr '/' '_' | sed 's/\.cpp$/.o/')"
  OBJECTS+=("$obj")
  if [[ -f "$obj" && "$obj" -nt "$src" ]]; then
    echo "  skip (up to date): $src"
    continue
  fi
  echo "  cc: $src"
  g++ -c "${CXXFLAGS[@]}" "$src" -o "$obj"
done

echo "=== [3] linking (--no-keep-memory) ==="
g++ "${OBJECTS[@]}" \
  $LLVM_LDFLAGS \
  -Wl,--no-keep-memory -Wl,--reduce-memory-overheads \
  -fopenmp -pthread \
  -lantlr4-runtime \
  -lPolly -lPollyISL -lisl \
  $LLVM_LIBS \
  $LLVM_SYSTEM_LIBS \
  -o GraphProgram

echo ">>> GraphProgram build complete"
