# Benchmark Comparison Analysis

- Successful cases: 70 / 72
- Average time p1 (no autotuner): 185.286 ms
- Average time p1-con: 545.714 ms
- Average time p2-con: 243.857 ms
- p1-con vs p1 speedup: -194.525%
- p2-con vs p1 speedup: -31.611%
- p2-con vs p1-con speedup: 55.314%
- Cases with switches in p1-con: 38
- Cases with switches in p2-con: 38
- Cases with switches in either autotuned implementation: 38
- Switched-case avg speedup (p1-con vs p1): -65.275%
- Switched-case avg speedup (p2-con vs p1): -81.716%
- Switched-case gain count (p1-con vs p1): 0 / 38
- Switched-case gain count (p2-con vs p1): 0 / 38

## Notes
- Positive speedup means faster than comparator.
- Conversion counts are from AutoTuner pass logs.
