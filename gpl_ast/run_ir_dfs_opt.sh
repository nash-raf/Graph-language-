./GraphProgram>test.ll test.graph

/usr/bin/llvm-link-18 myprog.bc dfs_runtime.bc bfs_runtime.bc bk_runtime.bc -o linked.bc

# Step 2: Optimize standard passes
opt-18 -O3 linked.bc -o linked.opt.bc

# Step 3: Run your custom passes along with standard function passes
opt -passes="function(mem2reg,loop-simplify,loop-rotate,lcssa,indvars,simplifycfg)" \
    linked.opt.bc -S -o linked-optimized.ll

# Step 4: Execute the optimized IR
lli-18 linked-optimized.ll