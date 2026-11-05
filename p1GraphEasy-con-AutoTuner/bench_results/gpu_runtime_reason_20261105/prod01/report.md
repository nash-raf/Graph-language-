# Final GPU runtime: end-to-end timing attribution

Five selected cases, three alternating uninstrumented/profiled fresh-process pairs per case, after one warmup per variant. Final correctness-checked runtime source and O3 flags. Instrumented output-validation programs passed independent full-vector checks. Production source, production objects and GPU kernel code are unchanged. All intermediate writes stayed on native Linux storage.

Buckets are exclusive wall time on the main thread, including time waiting for CPU workers or device completion inside each call. They sum to profiled whole-process wall time. CUDA API time includes blocking; transfer-call time is not pure PCIe bandwidth. Setup/finalization includes lifecycle callbacks and result publication, so it is not all disposable bookkeeping. Other host/input/runtime can include useful CPU fallback algorithm work.

Outside instrumented main includes executable/library loading, exit and profile report I/O. It was not attributed specifically to CUDA teardown. Device event intervals are separate, overlap the wall-time buckets, and include stream scheduling/submission gaps; they are not isolated arithmetic time.

| Case | Uninstrumented process ms | Profiled process ms | Difference |
|---|---:|---:|---:|
| bfs, soc/soc-slashdot.edges | 522.34 | 463.93 | -11.2% |
| pagerank, dimacs/hamming8-2.edges | 445.95 | 440.11 | -1.3% |
| pagerank, soc/soc-slashdot.edges | 794.58 | 828.53 | +4.3% |
| cc, dimacs10/fe-4elt2.edges | 640.23 | 637.38 | -0.4% |
| cc, dimacs10/inf-luxembourg_osm.edges | 4946.73 | 4958.96 | +0.2% |

## Exclusive shares of whole-process time

| Case | CUDA startup/module | CPU setup/finalize | Transfers | Host staging/coherence | Alloc/streams | Submit/complete | Other host/input/runtime | Outside main |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| bfs, soc/soc-slashdot.edges | 53.8% | 13.4% | 3.9% | 7.2% | 1.7% | 0.7% | 1.5% | 17.7% |
| pagerank, dimacs/hamming8-2.edges | 69.0% | 1.6% | 4.2% | 1.2% | 0.6% | 2.1% | 1.0% | 20.3% |
| pagerank, soc/soc-slashdot.edges | 28.0% | 18.2% | 4.0% | 6.0% | 0.5% | 7.1% | 26.1% | 10.2% |
| cc, dimacs10/fe-4elt2.edges | 34.5% | 22.1% | 23.3% | 3.5% | 1.0% | 3.1% | 1.3% | 11.2% |
| cc, dimacs10/inf-luxembourg_osm.edges | 4.4% | 41.4% | 39.4% | 7.5% | 0.4% | 3.4% | 2.0% | 1.6% |

## Detailed observations

bfs / soc/soc-slashdot.edges
- CUDA startup/module: 249.63 ms (53.8%)
- CPU setup/finalization: 62.15 ms (13.4%)
- Transfer API calls: 18.25 ms (3.9%)
- Host staging/coherence: 33.51 ms (7.2%)
- Device allocation/streams: 7.87 ms (1.7%)
- Submission/completion: 3.20 ms (0.7%)
- Other host/input/runtime: 7.14 ms (1.5%)
- Outside instrumented main: 82.19 ms (17.7%)
- GPU launches: 4. Separately measured stream event interval: 2.06 ms; overlaps the wall-time buckets.

pagerank / dimacs/hamming8-2.edges
- CUDA startup/module: 303.87 ms (69.0%)
- CPU setup/finalization: 7.14 ms (1.6%)
- Transfer API calls: 18.33 ms (4.2%)
- Host staging/coherence: 5.18 ms (1.2%)
- Device allocation/streams: 2.76 ms (0.6%)
- Submission/completion: 9.46 ms (2.1%)
- Other host/input/runtime: 4.21 ms (1.0%)
- Outside instrumented main: 89.15 ms (20.3%)
- GPU launches: 21. Separately measured stream event interval: 5.84 ms; overlaps the wall-time buckets.

pagerank / soc/soc-slashdot.edges
- CUDA startup/module: 231.71 ms (28.0%)
- CPU setup/finalization: 150.59 ms (18.2%)
- Transfer API calls: 32.94 ms (4.0%)
- Host staging/coherence: 49.88 ms (6.0%)
- Device allocation/streams: 4.38 ms (0.5%)
- Submission/completion: 58.94 ms (7.1%)
- Other host/input/runtime: 215.95 ms (26.1%)
- Outside instrumented main: 84.14 ms (10.2%)
- GPU launches: 21. Separately measured stream event interval: 50.20 ms; overlaps the wall-time buckets.

cc / dimacs10/fe-4elt2.edges
- CUDA startup/module: 220.13 ms (34.5%)
- CPU setup/finalization: 141.07 ms (22.1%)
- Transfer API calls: 148.52 ms (23.3%)
- Host staging/coherence: 22.39 ms (3.5%)
- Device allocation/streams: 6.08 ms (1.0%)
- Submission/completion: 19.44 ms (3.1%)
- Other host/input/runtime: 8.28 ms (1.3%)
- Outside instrumented main: 71.46 ms (11.2%)
- GPU launches: 92. Separately measured stream event interval: 3.30 ms; overlaps the wall-time buckets.

cc / dimacs10/inf-luxembourg_osm.edges
- CUDA startup/module: 215.91 ms (4.4%)
- CPU setup/finalization: 2053.72 ms (41.4%)
- Transfer API calls: 1952.70 ms (39.4%)
- Host staging/coherence: 369.75 ms (7.5%)
- Device allocation/streams: 19.15 ms (0.4%)
- Submission/completion: 166.16 ms (3.4%)
- Other host/input/runtime: 101.66 ms (2.0%)
- Outside instrumented main: 79.90 ms (1.6%)
- GPU launches: 750. Separately measured stream event interval: 27.63 ms; overlaps the wall-time buckets.

These are selected-case diagnostics, not a new full-corpus speed comparison. Profiling changes execution overhead; the paired uninstrumented/profiled differences above bound how literally to read the attributed shares. Use the uninstrumented primary benchmark for end-to-end speedup claims.
