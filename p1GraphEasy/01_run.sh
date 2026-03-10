# Compile the wrapper
gcc -c -O2 wrapper.c -o wrapper.o

# gcc -c -O2 runtime.c -o runtime11.o
g++ -c roaring_bitmap.cpp -O3 -mavx2 -o roaring_bitmap.o
# Compile the parallel runtime
gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

ld -r wrapper.o parallel_runtime.o -o runtime.o

