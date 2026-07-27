# opt-18 -passes="mem2reg,loop-simplify,loop-rotate,lcssa,indvars,simplifycfg" linked.bc -o linked.preopt.bc
# llvm-dis-18 linked.preopt.bc -o linked.preopt.ll

./GraphProgram>test.ll test.graph

/usr/bin/llvm-link-18 myprog.bc bfs_runtime.bc -o linked.bc


# Compile with profile instrumentation
clang++-18 linked.bc -fprofile-instr-generate -o linked.pgo.exe -lstdc++ -lpthread -lm

# Run program to collect profile
LLVM_PROFILE_FILE="profile.profraw" ./linked.pgo.exe edgelist.txt

# Merge raw profile data
llvm-profdata-18 merge -o profile.profdata profile.profraw

# Run Polly optimizations on LLVM IR
opt-18 -polly-canonicalize -polly-detect -polly-opt-isl -polly-codegen linked.bc -o linked.polly.bc

# Compile final executable with PGO
# clang++-18 linked.polly.bc -fprofile-instr-use=profile.profdata -O3 -o linked.final.exe -lstdc++ -lpthread -lm

clang++-18 linked.polly.bc -O3 -o linked.final.exe -lstdc++ -lpthread -lm

# Run final optimized program
./linked.final.exe edgelist.txt



# opt-18 -passes=dot-cfg linked.pgo.bc -disable-output
# llvm-profdata-18 show profile.profdata
# dot -Tpng .main.dot -o main.png
# dot -Tpng .bfs_runtime.dot -o bfs_runtime.png
