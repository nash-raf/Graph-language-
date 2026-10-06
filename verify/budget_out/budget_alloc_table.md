# Universal thread-budget allocation sweep

- level budget: 16 threads, level width 2 (plan entries 2), loop budget 14 split between two loops
- threads=16 rounds=40 trips=1048576 ratio(heavy:light)=3 reps=3 (best of)
- model chose light=7 heavy=7; measured optimum light=2 heavy=12 (7304.04 ms)

| light(101) | heavy(102) | time (ms) | ms/round | light workers | heavy workers | note |
|---|---|---|---|---|---|---|
| 1 | 13 | 7814.76 | 195.369 | 1 | 13 |  |
| 2 | 12 | 7304.04 | 182.601 | 2 | 12 | measured optimum |
| 3 | 11 | 10794.46 | 269.862 | 3 | 11 |  |
| 4 | 10 | 10995.21 | 274.880 | 4 | 10 |  |
| 5 | 9 | 10668.96 | 266.724 | 5 | 9 |  |
| 6 | 8 | 11827.84 | 295.696 | 6 | 8 |  |
| 7 | 7 | 11854.72 | 296.368 | 7 | 7 | **model choice** |
| 8 | 6 | 10330.09 | 258.252 | 8 | 6 |  |
| 9 | 5 | 11699.83 | 292.496 | 9 | 5 |  |
| 10 | 4 | 10576.04 | 264.401 | 10 | 4 |  |
| 11 | 3 | 11570.89 | 289.272 | 11 | 3 |  |
| 12 | 2 | 11648.03 | 291.201 | 12 | 2 |  |
| 13 | 1 | 7368.59 | 184.215 | 13 | 1 |  |
