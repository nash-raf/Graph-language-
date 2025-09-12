 ./GraphProgram>test.ll test.graph

# version
# /usr/bin/llvm-link-18  myprog.bc floyd_runtime.bc dfs_runtime.bc bfs_runtime.bc bk_runtime.bc -o linked.bc

# # Step 2: Optimize standard passes
# opt-18 -O3 linked.bc -o linked.opt.bc

# # Step 3: Run your custom passes along with standard function passes
# opt -passes="function(mem2reg,loop-simplify,loop-rotate,lcssa,indvars,simplifycfg)" \
#     linked.opt.bc -S -o linked-optimized.ll

# # Step 4: Execute the optimized IR
# lli-18 linked-optimized.ll


# Step 1: Link BCs
#llvm-link-18 myprog.bc dfs_runtime.bc bfs_single.bc bk_runtime.bc -o linked.bc
llvm-link-18 myprog.bc chromacity_runtime.bc floyd_runtime.bc dfs_runtime.bc bfs_runtime.bc bk_runtime.bc -o linked.bc

# Step 2: Optimize
opt-18 -O3 linked.bc -o linked.opt.bc

# Step 3: Compile to object
llc-18 -filetype=obj linked.opt.bc -o linked.o

# Step 4: Link to native binary with OpenMP
clang++-18 linked.o -fopenmp -o GraphProgram1

./GraphProgram1