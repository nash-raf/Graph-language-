# DOALL eta: predicted vs measured (pod, CPU backend)

pred_serial = c_ns * N;  pred_P = L_total(P) + c_ns*N/P;  measured_P = wall/rounds;  eta = pred/measured (1.00x = perfect)

| graph | N | P | pred_serial ms | pred_P ms | measured ms | eta | choose |
|---|---|---|---|---|---|---|---|
| bio-grid-yeast.txt | 6008 | 2 | 0.7 | 0.3 | 3.1 | 0.111 | parallel |
| bio-grid-yeast.txt | 6008 | 4 | 0.8 | 0.2 | 4.0 | 0.052 | parallel |
| bio-grid-yeast.txt | 6008 | 8 | 0.6 | 0.1 | 4.0 | 0.020 | parallel |
| g20k.txt | 20000 | 2 | 1.8 | 0.9 | 4.7 | 0.192 | parallel |
| g20k.txt | 20000 | 4 | 1.7 | 0.4 | 3.9 | 0.113 | parallel |
| g20k.txt | 20000 | 8 | 1.7 | 0.2 | 4.0 | 0.055 | parallel |
| ia-dbpedia.txt | 365493 | 2 | 31.0 | 15.5 | 16.1 | 0.963 | parallel |
| ia-dbpedia.txt | 365493 | 4 | 31.1 | 7.8 | 17.1 | 0.454 | parallel |
| ia-dbpedia.txt | 365493 | 8 | 31.0 | 3.9 | 15.5 | 0.251 | parallel |
