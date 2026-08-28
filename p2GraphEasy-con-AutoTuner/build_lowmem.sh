#!/usr/bin/env bash
# Low-memory rebuild of p2-AT's GraphProgram, mirroring p2GraphEasy's script:
# one translation unit at a time, no debug info, memory-tuned link.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"; cd "$ROOT"
LLVM_CONFIG=/usr/local/llvm-20-polly-rtti/bin/llvm-config
OBJ_DIR="${OBJ_DIR:-$ROOT/build_lowmem}"; mkdir -p "$OBJ_DIR"
LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"; LLVM_CXXFLAGS="${LLVM_CXXFLAGS//-fno-exceptions/}"
LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"; LLVM_LIBS="$($LLVM_CONFIG --libs all)"
LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"
# -iquote (not -I.) so #include <random> cannot resolve to the stray ELF file
# named `random` that sits in these trees.
CXXFLAGS=(-O2 -mavx2 -march=native -std=c++17 -fopenmp -fexceptions -pthread
          -I/usr/local/include/antlr4-runtime -Igenerated -iquote . $LLVM_CXXFLAGS)
SOURCES=(main.cpp IRGenVisitor.cpp MotifPattern.cpp MotifIRBuilder.cpp ASTBuilder.cpp SemanticAnalyzer.cpp AutoTunerPass.cpp
         roaring_bitmap.cpp generated/BaseBaseListener.cpp generated/BaseBaseVisitor.cpp
         generated/BaseLexer.cpp generated/BaseListener.cpp generated/BaseParser.cpp
         generated/BaseVisitor.cpp)
echo "=== [1] runtime.c ==="; gcc -c -O2 runtime.c -o "$OBJ_DIR/runtime.o"
echo "=== [2] compiling ${#SOURCES[@]} sources ==="
OBJECTS=("$OBJ_DIR/runtime.o")
for src in "${SOURCES[@]}"; do
  obj="$OBJ_DIR/$(echo "$src" | tr '/' '_' | sed 's/\.cpp$/.o/')"; OBJECTS+=("$obj")
  # Rebuild when the source OR any local header is newer.  Comparing against
  # the .cpp alone let a changed IRGenVisitor.h leave main.cpp.o compiled
  # against the previous class layout, which crashes at run time rather than
  # failing to link.
  stale=0
  [[ -f "$obj" ]] || stale=1
  if [[ "$stale" == 0 ]]; then
    for hdr in *.h; do [[ "$hdr" -nt "$obj" ]] && { stale=1; break; }; done
  fi
  [[ "$src" -nt "$obj" ]] && stale=1
  if [[ "$stale" == 0 ]]; then echo "  skip: $src"; continue; fi
  echo "  cc: $src"; g++ -c "${CXXFLAGS[@]}" "$src" -o "$obj"
done
echo "=== [3] linking ==="
g++ "${OBJECTS[@]}" $LLVM_LDFLAGS -Wl,--no-keep-memory -Wl,--reduce-memory-overheads \
  -fopenmp -pthread -lantlr4-runtime -lPolly -lPollyISL -lisl \
  $LLVM_LIBS $LLVM_SYSTEM_LIBS -o GraphProgram

# ---------------------------------------------------------------------------
# [4] Objects linked into COMPILED USER PROGRAMS (not into GraphProgram).
#
# autotuner_runtime.ll is linked into the user's module as IR by main.cpp, so it
# has to be regenerated whenever autotuner_runtime.c changes -- a stale .ll is
# silent: the graph never registers, and every autograph_edgemap step returns
# its input unchanged instead of erroring.  clang, not gcc, produces the .ll.
# roaring_bitmap.o needs -mavx2 -march=native.
# ---------------------------------------------------------------------------
echo "=== [4] user-program runtime objects ==="
CLANG="$(dirname "$LLVM_CONFIG")/clang"
[[ -x "$CLANG" ]] || CLANG=clang
echo "  ll: autotuner_runtime.c -> autotuner_runtime.ll"
"$CLANG" -O2 -fopenmp -iquote . -S -emit-llvm autotuner_runtime.c -o autotuner_runtime.ll
for rt in parallel_runtime_shim semiring_runtime; do
  echo "  cc: $rt.c"
  gcc -O3 -fopenmp -iquote . -c "$rt.c" -o "$rt.o"
done
for rt in runtime graph_runtime graph_loader_runtime_mmap; do
  src=""
  [[ -f "$rt.c"   ]] && src="$rt.c"
  [[ -f "$rt.cpp" ]] && src="$rt.cpp"
  [[ -n "$src" ]] || continue
  echo "  cc: $src"
  if [[ "$src" == *.cpp ]]; then g++ -O3 -fopenmp -iquote . -c "$src" -o "$rt.o"
  else gcc -O3 -fopenmp -iquote . -c "$src" -o "$rt.o"; fi
done
echo "  cc: roaring_bitmap.cpp (avx2)"
g++ -O3 -mavx2 -march=native -fopenmp -iquote . -c roaring_bitmap.cpp -o roaring_bitmap.o

echo ">>> p2-AT GraphProgram build complete"
echo ">>> link user programs with:"
echo "    g++ -O3 -fopenmp -no-pie program.o runtime.o roaring_bitmap.o \\"
echo "        graph_loader_runtime_mmap.o graph_runtime.o \\"
echo "        parallel_runtime_shim.o semiring_runtime.o -o prog"
