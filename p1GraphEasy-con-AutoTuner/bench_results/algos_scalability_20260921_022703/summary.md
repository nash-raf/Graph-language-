# Algorithm scalability — effect algebra + composable runtime

- build: 20260921_022703, seed 42, rounds 3 (+1 warmup), threads [1, 2, 4, 6, 8, 10, 12]
- size sweep: rewrite ON at 12 threads, rewrite OFF at 1 thread
- `on_kernel/off_kernel` subtract the load-only baseline at the same thread count; `speedup_kernel = off_kernel / on_kernel`
- thread sweep configs: `default` = as shipped (`SGPL_CLEANCUT_PARTITIONS` unset → workers*4, `SGPL_LOOP_CHUNK` unset → 256); `tuned` = `SGPL_LOOP_CHUNK=4`, `SGPL_CLEANCUT_PARTITIONS=<threads>`

## Size scaling (kernel ms)

| algo | size | ON ms | OFF ms | speedup | load ON ms | checksum |
|---|---|---|---|---|---|---|
| cc | 125k | 100 | 82.1 | 0.82x | 23.2 | OK |
| cc | 250k | 227 | 220 | 0.97x | 28.7 | OK |
| cc | 500k | 476 | 421 | 0.89x | 40.4 | OK |
| cc | 1M | 949 | 879 | 0.93x | 68.7 | OK |
| kcore | 125k | 58.8 | 11.1 | 0.19x | 23.2 | OK |
| kcore | 250k | 108 | 29.9 | 0.28x | 28.7 | OK |
| kcore | 500k | 229 | 67.4 | 0.30x | 40.4 | OK |
| kcore | 1M | 515 | 147 | 0.29x | 68.7 | OK |
| bfs_parent | 125k | 10.0 | 11.0 | 1.10x | 23.2 | OK |
| bfs_parent | 250k | 11.4 | 2.5 | 0.22x | 28.7 | OK |
| bfs_parent | 500k | 38.6 | 74.8 | 1.94x | 40.4 | OK |
| bfs_parent | 1M | 72.6 | 230 | 3.17x | 68.7 | OK |
| pagerank | 125k | 66.8 | 158 | 2.36x | 23.2 | OK |
| pagerank | 250k | 133 | 321 | 2.41x | 28.7 | OK |
| pagerank | 500k | 256 | 806 | 3.14x | 40.4 | OK |
| pagerank | 1M | 791 | 1890 | 2.39x | 68.7 | OK |
| sssp | 125k | 79.9 | 83.7 | 1.05x | 24.3 | OK |
| sssp | 250k | 171 | 160 | 0.94x | 32.2 | OK |
| sssp | 500k | 398 | 358 | 0.90x | 52.7 | OK |
| sssp | 1M | 927 | 874 | 0.94x | 84.4 | OK |
| pagerank_dataidx | 125k | 68.1 | 164 | 2.40x | 23.2 | OK |
| pagerank_dataidx | 250k | 142 | 344 | 2.41x | 28.7 | OK |
| pagerank_dataidx | 500k | 282 | 691 | 2.45x | 40.4 | OK |
| pagerank_dataidx | 1M | 689 | 1538 | 2.23x | 68.7 | OK |
| bfs_set | 125k | 44.9 | 16.2 | 0.36x | 23.2 | OK |
| bfs_set | 250k | 38.8 | 2.0 | 0.05x | 28.7 | OK |
| bfs_set | 500k | 170 | 86.5 | 0.51x | 40.4 | OK |
| bfs_set | 1M | 325 | 177 | 0.55x | 68.7 | OK |

## Thread scaling at n=1000000 — default (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.08x | 1.10x | 1.12x | 1.11x | 1.10x | 1.05x |
| kcore | 1.00x | 0.89x | 0.86x | 0.81x | 0.78x | 0.74x | 0.67x |
| bfs_parent | 1.00x | 1.59x | 2.17x | 2.17x | 2.34x | 2.63x | 2.10x |
| pagerank | 1.00x | 0.95x | 0.86x | 0.78x | 0.71x | 0.66x | 0.57x |
| sssp | 1.00x | 0.84x | 0.74x | 0.68x | 0.69x | 0.70x | 0.65x |
| pagerank_dataidx | 1.00x | 0.92x | 0.87x | 0.75x | 0.71x | 0.62x | 0.54x |
| bfs_set | 1.00x | 0.97x | 0.97x | 0.95x | 0.95x | 0.96x | 0.93x |

## Thread scaling at n=1000000 — tuned (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.01x | 1.03x | 1.16x | 1.19x | 1.26x | 1.36x |
| kcore | 1.00x | 0.90x | 0.87x | 0.89x | 0.91x | 0.87x | 0.86x |
| bfs_parent | 1.00x | 1.54x | 2.10x | 1.97x | 2.28x | 2.00x | 2.02x |
| pagerank | 1.00x | 0.96x | 0.95x | 1.02x | 1.15x | 1.14x | 1.20x |
| sssp | 1.00x | 0.91x | 0.87x | 0.94x | 0.98x | 1.05x | 1.10x |
| pagerank_dataidx | 1.00x | 0.97x | 0.84x | 1.04x | 1.21x | 1.14x | 1.23x |
| bfs_set | 1.00x | 0.89x | 0.86x | 0.90x | 0.95x | 1.01x | 1.03x |
