# Algorithm scalability — effect algebra + composable runtime

- build: 20260921_031033, seed 42, rounds 3 (+1 warmup), threads [1, 2, 4, 6, 8, 10, 12]
- size sweep: rewrite ON at 12 threads, rewrite OFF at 1 thread
- `on_kernel/off_kernel` subtract the load-only baseline at the same thread count; `speedup_kernel = off_kernel / on_kernel`
- thread sweep configs: `default` = as shipped (`SGPL_CLEANCUT_PARTITIONS` unset → workers*4, `SGPL_LOOP_CHUNK` unset → 256); `tuned` = `SGPL_LOOP_CHUNK=4`, `SGPL_CLEANCUT_PARTITIONS=<threads>`

## Size scaling (kernel ms)

| algo | size | ON ms | OFF ms | speedup | load ON ms | checksum |
|---|---|---|---|---|---|---|
| cc | 125k | 75.3 | 71.3 | 0.95x | 23.8 | OK |
| cc | 250k | 156 | 202 | 1.30x | 30.6 | OK |
| cc | 500k | 285 | 344 | 1.21x | 48.0 | OK |
| cc | 1M | 615 | 907 | 1.47x | 68.1 | OK |
| kcore | 125k | 56.9 | 16.4 | 0.29x | 23.8 | OK |
| kcore | 250k | 108 | 32.8 | 0.30x | 30.6 | OK |
| kcore | 500k | 201 | 62.9 | 0.31x | 48.0 | OK |
| kcore | 1M | 414 | 165 | 0.40x | 68.1 | OK |
| bfs_parent | 125k | 12.2 | 13.4 | 1.10x | 23.8 | OK |
| bfs_parent | 250k | 10.3 | 3.1 | 0.30x | 30.6 | OK |
| bfs_parent | 500k | 38.2 | 93.5 | 2.44x | 48.0 | OK |
| bfs_parent | 1M | 65.1 | 222 | 3.41x | 68.1 | OK |
| pagerank | 125k | 44.1 | 144 | 3.27x | 23.8 | OK |
| pagerank | 250k | 81.5 | 304 | 3.73x | 30.6 | OK |
| pagerank | 500k | 158 | 650 | 4.10x | 48.0 | OK |
| pagerank | 1M | 390 | 1831 | 4.69x | 68.1 | OK |
| sssp | 125k | 45.2 | 76.1 | 1.69x | 24.6 | OK |
| sssp | 250k | 79.6 | 154 | 1.94x | 33.5 | OK |
| sssp | 500k | 171 | 357 | 2.09x | 49.7 | OK |
| sssp | 1M | 394 | 848 | 2.15x | 78.8 | OK |
| pagerank_dataidx | 125k | 52.3 | 160 | 3.05x | 23.8 | OK |
| pagerank_dataidx | 250k | 91.3 | 332 | 3.64x | 30.6 | OK |
| pagerank_dataidx | 500k | 164 | 645 | 3.93x | 48.0 | OK |
| pagerank_dataidx | 1M | 398 | 1679 | 4.22x | 68.1 | OK |
| bfs_set | 125k | 35.3 | 18.7 | 0.53x | 23.8 | OK |
| bfs_set | 250k | 38.1 | 3.2 | 0.08x | 30.6 | OK |
| bfs_set | 500k | 117 | 80.6 | 0.69x | 48.0 | OK |
| bfs_set | 1M | 228 | 206 | 0.91x | 68.1 | OK |

## Thread scaling at n=1000000 — default (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.16x | 1.41x | 1.51x | 1.53x | 1.53x | 1.52x |
| kcore | 1.00x | 0.98x | 1.00x | 0.93x | 0.83x | 0.81x | 0.71x |
| bfs_parent | 1.00x | 1.55x | 2.10x | 2.59x | 2.60x | 2.56x | 2.44x |
| pagerank | 1.00x | 1.29x | 1.26x | 1.32x | 1.13x | 1.03x | 0.94x |
| sssp | 1.00x | 1.18x | 1.38x | 1.47x | 1.53x | 1.57x | 1.48x |
| pagerank_dataidx | 1.00x | 1.19x | 1.33x | 1.30x | 1.21x | 1.12x | 0.84x |
| bfs_set | 1.00x | 1.16x | 1.34x | 1.40x | 1.42x | 1.41x | 1.36x |

## Thread scaling at n=1000000 — tuned (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.11x | 1.37x | 1.42x | 1.50x | 1.53x | 1.54x |
| kcore | 1.00x | 1.02x | 1.00x | 1.01x | 0.98x | 0.96x | 0.95x |
| bfs_parent | - | - | - | - | - | - | - |
| pagerank | 1.00x | 1.33x | 1.62x | 1.63x | 1.62x | 1.51x | 1.36x |
| sssp | 1.00x | 1.02x | 1.19x | 1.24x | 1.31x | 1.30x | 1.22x |
| pagerank_dataidx | 1.00x | 1.42x | 1.63x | 1.65x | 1.50x | 1.52x | 1.43x |
| bfs_set | 1.00x | 1.01x | 1.13x | 1.16x | 1.20x | 1.20x | 1.22x |
