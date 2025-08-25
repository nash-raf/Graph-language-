./GraphProgram_try>test.ll test.graph
clang++-18 -fopenmp -std=c++17 myprog.bc bfs_runtime.bc -o test-exe -lomp -Wno-override-module

./test-exe


# clang++-18 -fopenmp -O2 -std=c++17 -emit-llvm -c bfs_runtime.cpp -o bfs_runtime.bc clang++-18 -fopenmp -std=c++17 -O2 -emit-llvm -c bfs_runtime.cpp -o bfs_runtime.bc -target x86_64-pc-linux-gnu
# clang++-18 -fopenmp -std=c++17 myprog.bc bfs_runtime.bc -o test-exe -lomp -target x86_64-pc-linux-gnu

# clang++-18 -fopenmp -O2 -std=c++17 -emit-llvm -c bfs_runtime.cpp -o bfs_runtime.bc -target x86_64-pc-linux-gnu

# clang++-18 -fopenmp -std=c++17 myprog.bc bfs_runtime.bc -o test-exe -lomp -Wno-override-module



./GraphProgram_try>test.ll test.graph
/usr/bin/llvm-link-18 myprog.bc bfs_runtime.bc -o linked.bc
lli-18 linked.bc

clang++-18 -fopenmp -std=c++17 myprog.bc bfs_runtime.bc -o test-exe -lomp


# clang++-18 -O2 -std=c++17 -emit-llvm -c bfs_runtime.cpp -o bfs_runtime.bc

#  clang++-18 -fopenmp -O2 -std=c++17 -emit-llvm -c bfs_runtime.cpp -o bfs_runtime.bc
