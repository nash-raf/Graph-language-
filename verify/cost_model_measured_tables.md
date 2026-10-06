## Measured economics

Controlled (repeated, warm off, median):

| family | case | units | work | cpu ms | gpu ms | cpu/gpu | gate default | gate correct |
|---|---|---|---|---|---|---|---|---|
| engine-step | bfs_level (activation forced) | arcs | 320000 | 186 | 458 | 0.41 | CPU | 1 |

Single pod runs (noisy; spread between repeats is the point):

| family | case | what ran on device | cpu ms | gpu ms | cpu/gpu | gate default | gate correct |
|---|---|---|---|---|---|---|---|
| doall | array_doall | outlined kernel | 198 | 162 | 1.22 | device | 1 |
| doall | array_doall (2nd run) | outlined kernel | 148 | 220 | 0.67 | device | - |
| doacross | doacross_carry | wave kernel | 219 | 179 | 1.22 | device | 1 |
| doacross | doacross_carry (2nd run) | wave kernel | 182 | 188 | 0.97 | device | - |
| mixed | mixed_regions | mixed kernel | 254 | 263 | 0.97 | device | - |
| mixed | mixed_regions (2nd run) | mixed kernel | 237 | 214 | 1.11 | device | 1 |
| doall | gpu_compute | outlined kernel | 176 | 183 | 0.96 | device | - |
| doall | gpu_compute (2nd run) | outlined kernel | 157 | 205 | 0.77 | device | - |
| engine-step | algo/kcore (default policy) | backend only (step declined) | 225 | 188 | 1.20 | CPU | 1 |
| engine-step | algo/kcore (2nd run) | backend only (step declined) | 200 | 244 | 0.82 | CPU | - |
| engine-step | algo/bfs_level (default policy) | backend only (step declined) | 214 | 187 | 1.14 | CPU | 1 |
| engine-step | algo/pagerank (default policy) | backend only (step declined) | 224 | 189 | 1.19 | CPU | 1 |
| engine-step | algo/pagerank (2nd run) | backend only (step declined) | 199 | 201 | 0.99 | CPU | - |
| engine-step | algo/cc (default policy) | backend only (step declined) | 204 | 167 | 1.22 | CPU | 1 |
| engine-step | algo/cc (2nd run) | backend only (step declined) | 179 | 157 | 1.14 | CPU | 1 |
