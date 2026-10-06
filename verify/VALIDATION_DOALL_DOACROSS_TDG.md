# Cost-model validation: DOALL, DOACROSS, TDG thread budget

Machine: runpod CPU (48 cores, 24 threads used), CPU backend, best of 3, wall time.

- DOALL subject: `doall_scaling` on g20k, N=20000, 200 rounds, site `task_1::foreach.cond16`.
  - predicted serial: `c_ns*N`;  predicted parallel: `L(P) + c_ns*N/P`
  - predicted/measured ratio eta = pred/measured, 1.00x = perfect. All rows `c_state=stable`, `choose=parallel`.
- DOACROSS subject: `doacross_repeat_d1` (distance-1 chain), N=8191, 200 dispatches.
  - predicted: `L + N*c_dep + N*c_ind/P + N*sync`; c_dep=30.7 ns/iter (stable).
  - the model decides **serial** (correct for a chain), so the comparable column is the unforced measurement; the forced-parallel column is that path measured directly (SGPL_FORCE_DOACROSS_PARALLEL=1).
- TDG budget: two parallelizable task loops in one level; level budget 24, level width 2, loop budget 22; every allocation enumerated with SGPL_FORCE_WIDTHS.

## 1. DOALL -- P = 1..24

| P | pred serial (ms) | pred P (ms) | measured (ms) | eta |
|---|---|---|---|---|
| 1 | 1.6962 | 1.6962 | 2.8622 | 0.593 |
| 2 | 1.6962 | 0.8497 | 2.8325 | 0.300 |
| 3 | 1.6962 | 0.5671 | 4.9889 | 0.114 |
| 4 | 1.6962 | 0.4260 | 4.7513 | 0.090 |
| 5 | 1.6962 | 0.3413 | 4.9862 | 0.068 |
| 6 | 1.6962 | 0.2850 | 4.7300 | 0.060 |
| 7 | 1.6962 | 0.2448 | 4.9845 | 0.049 |
| 8 | 1.6962 | 0.2147 | 4.9994 | 0.043 |
| 9 | 1.6962 | 0.1913 | 4.7453 | 0.040 |
| 10 | 1.6962 | 0.1726 | 4.9729 | 0.035 |
| 11 | 1.6962 | 0.1574 | 5.0139 | 0.031 |
| 12 | 1.6962 | 0.1447 | 4.7272 | 0.031 |
| 13 | 1.6962 | 0.1340 | 4.9738 | 0.027 |
| 14 | 1.6962 | 0.1249 | 4.9699 | 0.025 |
| 15 | 1.6962 | 0.1170 | 4.9924 | 0.023 |
| 16 | 1.6962 | 0.1101 | 4.9838 | 0.022 |
| 17 | 1.6962 | 0.1040 | 4.9727 | 0.021 |
| 18 | 1.6962 | 0.0987 | 4.7063 | 0.021 |
| 19 | 1.6962 | 0.0939 | 5.0279 | 0.019 |
| 20 | 1.6962 | 0.0896 | 4.9778 | 0.018 |
| 21 | 1.6962 | 0.0858 | 4.7179 | 0.018 |
| 22 | 1.6962 | 0.0823 | 5.0071 | 0.016 |
| 23 | 1.6962 | 0.0791 | 5.0005 | 0.016 |
| 24 | 1.6962 | 0.0762 | 4.9838 | 0.015 |

## 2. DOACROSS -- P = 1..24

| P | pred P (ms) | measured unforced/serial (ms) | eta serial | measured forced-parallel (ms) |
|---|---|---|---|---|
| 1 | 0.2520 | 0.6335 | 0.398 | 4.6767 |
| 2 | 0.2714 | 0.6336 | 0.428 | 6.7618 |
| 3 | 0.2521 | 0.6361 | 0.396 | 6.4841 |
| 4 | 0.2515 | 0.6372 | 0.395 | 5.9024 |
| 5 | 0.2523 | 0.6278 | 0.402 | 6.3685 |
| 6 | 0.2530 | 0.6417 | 0.394 | 5.1473 |
| 7 | 0.2509 | 0.6475 | 0.387 | 6.3775 |
| 8 | 0.2515 | 0.6293 | 0.400 | 4.6968 |
| 9 | 0.2549 | 0.6507 | 0.392 | 5.2569 |
| 10 | 0.2517 | 0.6529 | 0.386 | 4.6992 |
| 11 | 0.2528 | 0.6324 | 0.400 | 6.8551 |
| 12 | 0.2517 | 0.6359 | 0.396 | 6.3989 |
| 13 | 0.2526 | 0.6377 | 0.396 | 5.3145 |
| 14 | 0.2510 | 0.6440 | 0.390 | 6.5231 |
| 15 | 0.2518 | 0.6372 | 0.395 | 6.4775 |
| 16 | 0.2514 | 0.6433 | 0.391 | 6.2970 |
| 17 | 0.2547 | 0.5964 | 0.427 | 6.4116 |
| 18 | 0.2539 | 0.6518 | 0.390 | 6.4463 |
| 19 | 0.2511 | 0.6384 | 0.393 | 4.1091 |
| 20 | 0.2533 | 0.6387 | 0.397 | 5.0495 |
| 21 | 0.2516 | 0.6475 | 0.389 | 6.8242 |
| 22 | 0.2509 | 0.6294 | 0.399 | 6.4059 |
| 23 | 0.2527 | 0.6354 | 0.398 | 4.4487 |
| 24 | 0.2533 | 0.6470 | 0.391 | 6.4703 |

## 3. TDG universal thread-budget -- every allocation (24 threads)

Level budget 24, level width 2 (one per task), loop budget 22 split between the two loops.
Model chose **11 + 11**; measured optimum **21 + 1**. Verdict: **MISMATCH**.

| light (101) | heavy (102) | time (ms) | ms/round | note |
|---|---|---|---|---|
| 1 | 21 | 10091.62 | 252.291 |  |
| 2 | 20 | 13680.11 | 342.003 |  |
| 3 | 19 | 13510.93 | 337.773 |  |
| 4 | 18 | 13707.61 | 342.690 |  |
| 5 | 17 | 14481.92 | 362.048 |  |
| 6 | 16 | 13315.91 | 332.898 |  |
| 7 | 15 | 14599.82 | 364.996 |  |
| 8 | 14 | 13758.30 | 343.957 |  |
| 9 | 13 | 14120.14 | 353.004 |  |
| 10 | 12 | 14212.47 | 355.312 |  |
| 11 | 11 | 13270.55 | 331.764 | **model choice** |
| 12 | 10 | 13293.38 | 332.335 |  |
| 13 | 9 | 14565.48 | 364.137 |  |
| 14 | 8 | 14590.29 | 364.757 |  |
| 15 | 7 | 13048.82 | 326.221 |  |
| 16 | 6 | 13912.08 | 347.802 |  |
| 17 | 5 | 14578.24 | 364.456 |  |
| 18 | 4 | 13599.19 | 339.980 |  |
| 19 | 3 | 13730.45 | 343.261 |  |
| 20 | 2 | 14052.07 | 351.302 |  |
| 21 | 1 | 9494.58 | 237.365 | measured optimum |

## What the numbers say

- DOALL at large work is accurate (ia-dbpedia N=365k: eta2 = 0.963 on the 16-thread run); on this small subject the measurement is a flat ~4.7-5.0 ms/round for P>=3 (a per-dispatch pool floor) while the model keeps predicting a 1/P decrease, so eta collapses to 0.02-0.09.
- DOACROSS for a chain: the serial decision is right (forcing parallel costs 7-10x), but the model's own serial cost is ~2.5x low (pred 0.25 ms vs measured 0.64 ms per dispatch), i.e. c_dep=30.7 ns/iter underestimates the real ~78 ns/iter.
- TDG budget: the model minimizes a **sum** of per-loop modelled costs, but the level runs its tasks concurrently (round = max of the two task times) and each dispatch pays the pool floor; the balanced 11+11 allocation it chooses is 40% slower than the measured optimum 21+1.

