
# sudo apt update
# sudo apt install -y libnlopt-dev

# gcc -c -O2 runtime.c -o runtime11.o
gcc -O2 -c parallel_runtime.c -o parallel_runtime.o
g++ -O3 -mavx2 -march=native -c roaring_bitmap.cpp -o roaring_bitmap.o

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o

# g++ -c -O2 -std=c++17 graph_loader_runtime.cpp -o graph_loader_runtime.o


# Compile the parallel runtime
gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

ld -r parallel_runtime.o -o runtime.o



# gcc -c -O2 wrapper.c -o wrapper.o

# g++ -std=c++17 -O3 -fopenmp -mavx2 -c roaring_bitmap.cpp -o roaring_bitmap.o


# =======
# # gcc -c -O2 runtime.c -o runtime11.o
# g++ -c roaring_bitmap.cpp -O3 -mavx2 -o roaring_bitmap.o

# # Compile the parallel runtime
# gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

# ld -r wrapper.o parallel_runtime.o -o runtime.o
