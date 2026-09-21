# Algorithm scalability — effect algebra + composable runtime

- build: 20260921_102832, seed 42, rounds 3 (+1 warmup), threads [1, 2, 4, 6, 8, 10, 12]
- size sweep: rewrite ON at 12 threads, rewrite OFF at 1 thread
- `on_kernel/off_kernel` subtract the load-only baseline at the same thread count; `speedup_kernel = off_kernel / on_kernel`
- thread sweep configs: `default` = as shipped (`SGPL_CLEANCUT_PARTITIONS` unset → workers*4, `SGPL_LOOP_CHUNK` unset → 256); `tuned` = `SGPL_LOOP_CHUNK=4`, `SGPL_CLEANCUT_PARTITIONS=<threads>`

## Size scaling (kernel ms)

| algo | size | ON ms | OFF ms | speedup | load ON ms | checksum |
|---|---|---|---|---|---|---|
| cc | 125k | 75.6 | 80.9 | 1.07x | 23.7 | OK |
| cc | 250k | 143 | 199 | 1.38x | 28.5 | OK |
| cc | 500k | 291 | 352 | 1.21x | 45.7 | OK |
| cc | 1M | 652 | 975 | 1.50x | 71.3 | OK |
| kcore | 125k | 43.2 | 17.1 | 0.40x | 23.7 | OK |
| kcore | 250k | 72.2 | 35.4 | 0.49x | 28.5 | OK |
| kcore | 500k | 137 | 80.5 | 0.59x | 45.7 | OK |
| kcore | 1M | 268 | 166 | 0.62x | 71.3 | OK |
| bfs_parent | 125k | 12.2 | 14.6 | 1.20x | 23.7 | OK |
| bfs_parent | 250k | 11.6 | 5.8 | 0.50x | 28.5 | OK |
| bfs_parent | 500k | 34.8 | 81.6 | 2.34x | 45.7 | OK |
| bfs_parent | 1M | 77.1 | 227 | 2.95x | 71.3 | OK |
| pagerank | 125k | 31.4 | 170 | 5.40x | 23.7 | OK |
| pagerank | 250k | 57.2 | 338 | 5.91x | 28.5 | OK |
| pagerank | 500k | 106 | 687 | 6.49x | 45.7 | OK |
| pagerank | 1M | 265 | 1760 | 6.64x | 71.3 | OK |
| sssp | 125k | 50.4 | 80.2 | 1.59x | 26.5 | OK |
| sssp | 250k | 92.0 | 157 | 1.71x | 28.6 | OK |
| sssp | 500k | 162 | 351 | 2.17x | 57.6 | OK |
| sssp | 1M | 386 | 889 | 2.30x | 83.2 | OK |
| pagerank_dataidx | 125k | 32.6 | 167 | 5.12x | 23.7 | OK |
| pagerank_dataidx | 250k | 64.7 | 349 | 5.39x | 28.5 | OK |
| pagerank_dataidx | 500k | 118 | 709 | 6.00x | 45.7 | OK |
| pagerank_dataidx | 1M | 299 | 1706 | 5.70x | 71.3 | OK |
| bfs_set | 125k | 36.6 | 19.8 | 0.54x | 23.7 | OK |
| bfs_set | 250k | 42.0 | 4.6 | 0.11x | 28.5 | OK |
| bfs_set | 500k | 125 | 90.7 | 0.73x | 45.7 | OK |
| bfs_set | 1M | 234 | 199 | 0.85x | 71.3 | OK |

## Thread scaling at n=1000000 — default (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.18x | 1.38x | 1.48x | 1.49x | 1.49x | 1.52x |
| kcore | 1.00x | 1.13x | 1.07x | 1.30x | 1.20x | 1.22x | 1.24x |
| bfs_parent | 1.00x | 1.59x | 2.29x | 2.39x | 2.83x | 2.91x | 2.52x |
| pagerank | 1.00x | 1.47x | 1.79x | 1.67x | 1.79x | 1.75x | 1.60x |
| sssp | 1.00x | 1.13x | 1.35x | 1.47x | 1.50x | 1.49x | 1.43x |
| pagerank_dataidx | 1.00x | 1.32x | 1.61x | 1.65x | 1.68x | 1.60x | 1.49x |
| bfs_set | 1.00x | 1.15x | 1.30x | 1.19x | 1.35x | 1.36x | 1.32x |

## Thread scaling at n=1000000 — tuned (kernel speedup vs 1 thread)

| algo | 1T | 2T | 4T | 6T | 8T | 10T | 12T |
|---|---|---|---|---|---|---|---|
| cc | 1.00x | 1.13x | 1.33x | 1.43x | 1.42x | 1.52x | 1.48x |
| kcore | 1.00x | 1.03x | 1.05x | 1.06x | 1.03x | 1.08x | 1.09x |
| bfs_parent | 1.00x | 1.55x | 2.09x | 2.33x | 2.33x | 2.57x | 2.49x |
| pagerank | 1.00x | 1.60x | 1.95x | 1.91x | 1.93x | 2.01x | 1.67x |
| sssp | 1.00x | 1.01x | 1.18x | 1.27x | 1.24x | 1.24x | 1.18x |
| pagerank_dataidx | 1.00x | 1.37x | 1.73x | 1.75x | 1.69x | 1.78x | 1.73x |
| bfs_set | 1.00x | 1.01x | 1.07x | 1.10x | 1.14x | 1.15x | 1.15x |
