# WACO vs ARS AutoTuner — region-level comparison

Regions compared: **5**

## Agreement (AutoTuner local_argmin vs WACO-G)
- match rate: 0.200
- Cohen's kappa: 0.167

## Accuracy vs measured oracle (kernel-only)
- AutoTuner top-1: (1.0, 1.0, 1.0)
- WACO-G top-1: (0.4, 0.0, 0.8)
- AutoTuner regret geomean: 1.0038060675554215
- WACO-G regret geomean: 1.0836953310105648
- McNemar: {'n01': 0, 'n10': 3, 'chi2': 1.3333333333333333, 'p_approx': 0.24821307898992362}

## Threats to validity
- WACO-G uses a WACONet-inspired CNN without MinkowskiEngine sparse convolution; results are methodological, not a bit-exact WACO reproduction.
- Corpus is synthetic (ER/BA/RMAT) resized for laptop GPU/CPU limits, smaller than WACO's 726 SuiteSparse test matrices.
- Track 2 format mapping from SuperSchedule level formats to CSR/BCSR is approximate.
- AutoTuner conversion costs in end-to-end DP use the compiler's analytic model; WACO-G DP uses a proxy conversion term.
- Hardware is AMD Ryzen 7 4800H; WACO paper used Xeon+icc — models are retrained here.
