#!/usr/bin/env bash
# Provision a fresh pod for the GPU port: apt deps, antlr4 4.13.2 runtime,
# LLVM 20.1.8 with NVPTX+Polly+RTTI, then rebuild GraphProgram with it.
set -u
LOG=/tmp/pod_setup.log
say() { echo "=== $*"; }
exec > >(tee -a "$LOG") 2>&1
say "apt deps"
apt-get update -qq && apt-get install -y -qq ninja-build gdb cmake curl unzip \
  libisl-dev libxml2-dev libzstd-dev libedit-dev libffi-dev zlib1g-dev libtinfo-dev libnlopt-dev
say "antlr4 4.13.2 runtime"
if [ ! -f /usr/local/include/antlr4-runtime/antlr4-runtime.h ]; then
  cd /tmp && curl -sL -o antlr.zip https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip \
    && unzip -q -o antlr.zip -d antlr_src \
    && cmake -S antlr_src -B antlr_build -G Ninja -DANTLR_BUILD_CPP_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build antlr_build -j40 && cmake --install antlr_build && ldconfig
fi
say "antlr: $(ls /usr/local/include/antlr4-runtime/antlr4-runtime.h 2>/dev/null || echo FAILED)"
say "LLVM 20.1.8 + NVPTX"
if [ ! -x /root/llvm-20.1.8-nvptx/bin/llvm-config ]; then
  cd /root && rm -rf llvm-project-20.1.8.src && cp /workspace/llvm-src/llvm-20.1.8.tar.xz . && tar xf llvm-20.1.8.tar.xz \
   && cmake -S llvm-project-20.1.8.src/llvm -B llvm-build-nvptx -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_PROJECTS=polly -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_DOCS=OFF -DLLVM_ENABLE_ASSERTIONS=OFF -DCMAKE_INSTALL_PREFIX=/root/llvm-20.1.8-nvptx \
   && ninja -C llvm-build-nvptx -j40 && ninja -C llvm-build-nvptx install
fi
say "llvm targets: $(/root/llvm-20.1.8-nvptx/bin/llvm-config --targets-built 2>/dev/null)"
say "GraphProgram rebuild"
P=/workspace/IMTalker/p1/Graph-language-/p1GraphEasy-con-AutoTuner
cd "$P" && rm -rf build_nvptx && OBJ_DIR="$P/build_nvptx" LLVM_CONFIG=/root/llvm-20.1.8-nvptx/bin/llvm-config bash build_lowmem.sh
say "GraphProgram: $(ls -la GraphProgram 2>/dev/null | awk '{print $5}') bytes"
say "SETUP-DONE"
