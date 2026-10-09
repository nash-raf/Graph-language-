#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
LLVM_CONFIG="${LLVM_CONFIG_BIN:-llvm-config-20}"
OBJ_DIR="${OBJ_DIR:-$ROOT/build_tdg_validation}"
mkdir -p "$OBJ_DIR"

read -r -a LLVM_FLAGS <<< "$($LLVM_CONFIG --cxxflags)"
read -r -a LLVM_LINK <<< "$($LLVM_CONFIG --ldflags)"
read -r -a LLVM_LIBS <<< "$($LLVM_CONFIG --libs all)"
read -r -a LLVM_SYSTEM <<< "$($LLVM_CONFIG --system-libs)"

SOURCES=(main.cpp IRGenVisitor.cpp MotifPattern.cpp MotifIRBuilder.cpp ASTBuilder.cpp
         pdg.cpp parallel_loop_outline.cpp graph_frontier_lowering.cpp
         SemanticAnalyzer.cpp roaring_bitmap.cpp AutoTunerPass.cpp
         generated/BaseBaseListener.cpp generated/BaseBaseVisitor.cpp
         generated/BaseLexer.cpp generated/BaseListener.cpp
         generated/BaseParser.cpp generated/BaseVisitor.cpp)

compile_one() {
  local source="$1" object="$2"
  g++ -w -O1 -std=c++17 -fexceptions -pthread -mavx2 -march=native \
    -I/usr/local/include/antlr4-runtime -Igenerated -iquote . \
    -I"$($LLVM_CONFIG --includedir)/polly" \
    "${LLVM_FLAGS[@]}" -fexceptions -c "$source" -o "$object"
}
objects=()
for source in "${SOURCES[@]}"; do
  object="$OBJ_DIR/${source//\//_}.o"
  objects+=("$object")
  if [[ ! -f "$object" || "$source" -nt "$object" || pdg.h -nt "$object" ||
        autotuner_runtime.h -nt "$object" ]]; then
    compile_one "$source" "$object"
  fi
done

g++ "${objects[@]}" "${LLVM_LINK[@]}" -pthread -L/usr/local/lib \
  -Wl,-rpath,/usr/local/lib -lantlr4-runtime \
  "${LLVM_LIBS[@]}" "${LLVM_SYSTEM[@]}" -o "$OBJ_DIR/GraphProgram_tdg"
echo "$OBJ_DIR/GraphProgram_tdg"
