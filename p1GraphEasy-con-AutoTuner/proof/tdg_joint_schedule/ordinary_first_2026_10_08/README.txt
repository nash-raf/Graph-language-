Immutable ordinary-first benchmark reference

This snapshot preserves the current working-tree runtime byte for byte, including changes made since the October 7 audit. It is not the older phased_sum intermediate.

The source/main.cpp copy records current compiler wiring only. Production files have not been edited. Compile the saved runtime as the sole parallel-runtime object for a reference executable; never link it alongside the candidate parallel runtime. Use identical compiler output, input, dependencies and build flags for both policies. The manifest records copied-source hashes and hashes of relevant compiler/dependency context.

The planned temporary main.cpp policy option will select the emitted TDG entry point, with the benchmark driver selecting the corresponding single runtime object. If ordinary-first wins, restore/integrate its scheduling path into the normal runtime and remove the temporary selection; production must not depend on this snapshot.
