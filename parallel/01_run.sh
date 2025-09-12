# Compile the wrapper
gcc -c -O2 wrapper.c -o wrapper.o

# Compile the parallel runtime
gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

ld -r wrapper.o parallel_runtime.o -o runtime.o
