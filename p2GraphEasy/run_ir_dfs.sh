./GraphProgram>test.ll test.graph

/usr/bin/llvm-link myprog.bc dfs_runtime.bc -o linked.bc
lli linked.bc