# Handoff: WACO vs ARS AutoTuner (full-scale experiment)

This guide is for running the **full-scale** region-level comparison of WACO
(ASPLOS’23) against the ARS AutoTuner in `p1GraphEasy-con-AutoTuner/`.

Read this once end-to-end before starting a long overnight run.

---

## 1. What you are measuring

**Question:** for each AutoTuner *decision region* (Traverse / Insert) and input
graph, do WACO-G and the AutoTuner pick the same layout, and which one matches
the measured best?

| Axis | Value |
|------|--------|
| Layouts (Track 1) | CSR, PCSR, BCSR, SET |
| Layouts (Track 2 overlap) | CSR vs BCSR only |
| Ground truth | Forced-layout builds; median of warm runs; ≤5% of fastest = “best” |
| Scoring | Kernel-only ranking + end-to-end layout DP |
| Hardware | **Your** machine (retrain WACO-G here; do not reuse Xeon/icc weights) |

Two tracks:

1. **WACO-G** — WACO’s *method* (CNN over sparsity + region embedder + ranking
   loss) on the AutoTuner’s 4-layout space. This is the main apple-to-apple
   comparison.
2. **Track 2** — CSR/BCSR overlap study for read-only traverse regions
   (SpMV-like). Optional artifact mapping if WACO top-k files exist.

---

## 2. Machine requirements

### Required

| Component | Notes |
|-----------|--------|
| Linux x86_64 | Tested on Fedora; Ubuntu should work with package-name tweaks |
| `g++`, `gcc`, `make`, `cmake` | C++17 |
| LLVM 20 with Polly | `llvm-config-20`, `libPolly` |
| ANTLR 4 C++ runtime + `antlr4` CLI | Needed to regenerate `generated/` (gitignored) |
| NLopt | Local tree under `p1GraphEasy-con-AutoTuner/.deps/nlopt` **or** system `libnlopt` |
| Python 3.10+ | `numpy`, `torch` (CPU is fine; GPU speeds training) |
| RAM | ≥16 GB recommended; 32 GB comfortable for larger graphs |
| Disk | ≥20 GB free (`data/`, builds, third_party) |

### Optional but useful

- NVIDIA GPU with working CUDA PyTorch — cuts WACO-G / Track-2 training time
- Cap graphs at ~2M edges if VRAM ≤4 GB (pipeline already defaults `max_edges=2e6`)

### Not required

- MinkowskiEngine (WACO-G uses a WACONet-*inspired* CNN fallback)
- Intel `icc` (use `gcc`; document that in the paper threats section)

---

## 3. One-time setup

```bash
git clone https://github.com/nash-raf/Graph-language-.git
cd Graph-language-
git checkout Waco-compare

# --- system packages (Fedora example) ---
# sudo dnf install gcc gcc-c++ llvm20 llvm20-devel llvm20-libs \
#   antlr4 antlr4-cpp-runtime antlr4-cpp-runtime-devel \
#   cmake python3-pip python3-numpy

# --- Python ---
python3 -m pip install --user 'torch' numpy  # or conda/micromamba env
python3 -c "import torch, numpy; print(torch.__version__)"

# --- NLopt (if .deps missing) ---
# Either install libnlopt-dev / nlopt-devel, or build into
# p1GraphEasy-con-AutoTuner/.deps/nlopt as the original machine did.

# --- regenerate ANTLR sources (generated/ is gitignored) ---
cd p1GraphEasy-con-AutoTuner
antlr4 -Dlanguage=Cpp -visitor -listener -o generated Base.g4

# Include path: Fedora uses /usr/include/antlr4-runtime
# (build_lowmem.sh on this branch already points there).
# On Ubuntu/Debian you may need:
#   -I/usr/include/antlr4-runtime   or  -I/usr/local/include/antlr4-runtime

export LLVM_CONFIG=llvm-config-20
export POLLY_LIBS="-lPolly -lPollyISL"   # if link fails looking for Polly
export GRAPH_DISABLE_POLLY=1             # Polly not needed for layout study
export FORCE_CPU=1                       # never emit GPU/PTX during comparison

bash ./build_lowmem.sh
test -x ./GraphProgram && echo "compiler OK"

# First-run hardware calibration for the analytic cost model
mkdir -p ~/.config/sgpl
# 03_run.sh / the harness will create hw_calib.json via hw_calib_bench if missing

# --- optional: upstream WACO artifact (Track 2 mapping only) ---
cd ../waco_compare
mkdir -p third_party
git clone --depth 1 https://github.com/nullplay/Workload-Aware-Co-Optimization.git \
  third_party/Workload-Aware-Co-Optimization
```

Smoke test (tiny):

```bash
cd waco_compare
LIMIT_GRAPHS=12 LIMIT_CASES=4 RUNS=2 EPOCHS=5 ./run_pipeline.sh
```

If that finishes and writes `results/comparison_report.md`, proceed to full scale.

---

## 4. Full-scale recommended settings

These are the settings intended for a paper-quality corpus on a desktop/workstation.

```bash
cd waco_compare
export FORCE_CPU=1 GRAPH_DISABLE_POLLY=1
export OMP_NUM_THREADS=8 SGPL_NUM_THREADS=8   # match your physical cores

# Full-scale (overnight / multi-day). Adjust to your machine.
LIMIT_GRAPHS=400 \
LIMIT_CASES=0 \
RUNS=5 \
EPOCHS=80 \
./run_pipeline.sh
```

| Knob | Pilot | Full-scale | Meaning |
|------|-------|------------|---------|
| `LIMIT_GRAPHS` | 48 | **200–400** | Graphs in corpus (`build_corpus.py --limit`) |
| `LIMIT_CASES` | 16 | **0** (= all instantiated workloads) | Cap on measure/predict cases |
| `RUNS` | 3 | **5** | Warm measured runs per layout (plus 1 warmup) |
| `EPOCHS` | 15–20 | **60–100** | WACO-G training epochs |
| Corpus reps | 2 | 2–3 | Replicates per size×family (edit `run_pipeline.sh` or call `build_corpus.py` directly) |

### What “full scale” costs (wall time, rough)

Assume 8 cores, ~100–150 ms–few seconds per small kernel, much longer for large graphs.

| Stage | Dominant cost | Order of magnitude |
|-------|---------------|--------------------|
| Corpus gen | CPU | Minutes |
| Instantiation | Disk | Minutes |
| **Ground truth** | 4 layouts × compile + run per case | **Hours → 1–2 days** for hundreds of cases |
| AutoTuner preds | Compile-only + dump | Hours |
| WACO-G train | Epochs × pairs | **Minutes (CPU) → ~1 h**; GPU faster |
| Track 2 + analysis | Small | Minutes |

**Bottleneck is almost always ground-truth measurement**, not neural training.

You can run stages separately (useful for resume / overnight):

```bash
python3 corpus/build_corpus.py --limit 400 --reps 2
python3 workloads/instantiate.py --split train
python3 workloads/instantiate.py --split test

# Longest step — run overnight; safe to stop and re-run with --limit
python3 measure_regions.py --split train --runs 5 --warmup 1 --out results/ground_truth_train.csv
python3 measure_regions.py --split test  --runs 5 --warmup 1 --out results/ground_truth_test.csv
# merge CSVs as in run_pipeline.sh

python3 autotuner_predictions.py --split test --out results/autotuner_pred.csv
python3 wacog/train.py --gt results/ground_truth.csv --epochs 80 --out results/wacog.pt
python3 wacog/predict.py --gt results/ground_truth.csv --ckpt results/wacog.pt \
  --out results/wacog_pred.csv --plans results/wacog_plans.json
python3 waco_artifact/run_track2.py --gt results/ground_truth.csv --epochs 40 \
  --out results/waco_artifact_pred.csv
python3 analysis/compare.py \
  --gt results/ground_truth.csv \
  --at results/autotuner_pred.csv \
  --wacog results/wacog_pred.csv \
  --art results/waco_artifact_pred.csv \
  --out results/comparison_report.json \
  --md results/comparison_report.md
```

---

## 5. Training: how long, and when to train more

### What is being trained

- **WACO-G** (`wacog/train.py`): pairwise hinge ranking over layout pairs per
  (graph, template, region). Loss should **trend down** across epochs.
- **Track 2** (`waco_artifact/run_track2.py`): small CSR-vs-BCSR network on
  traverse-only regions.

The AutoTuner itself is **not** trained; it uses the analytic cost model +
`~/.config/sgpl/hw_calib.json` measured on *your* CPU.

### Reading the training log

```text
epoch 12/80 loss=0.91
epoch 13/80 loss=0.88
...
```

| Signal | Interpretation | Action |
|--------|----------------|--------|
| Loss flat after ~10–20 epochs and still ~1.0 | Not learning / too little data | Collect more GT; check only Traverse/Insert rows with 4 layouts |
| Loss steadily falling then plateaus | Normal | Stop; evaluate |
| Train top-1 ≫ test top-1 | Overfit | More graphs; early stop; fewer epochs |
| Test top-1 rising with more epochs | Underfit | Train more (e.g. 80 → 120) |
| Pilot: ~5–20 regions, top-1 noisy | Underpowered | Scale corpus — don’t over-interpret |

### Practical schedule

1. **Smoke:** `EPOCHS=5`, tiny corpus — proves the path.
2. **Pilot:** `LIMIT_GRAPHS=48`, `LIMIT_CASES=24`, `EPOCHS=20` — sanity metrics.
3. **Full:** `LIMIT_GRAPHS≥200`, all cases, `EPOCHS=80`, `RUNS=5`.
4. **Train more only if** validation/test top-1 or Kendall τ is still climbing
   between checkpoints. Save checkpoints:

```bash
python3 wacog/train.py --gt results/ground_truth.csv --epochs 40 --out results/wacog_e40.pt
python3 wacog/train.py --gt results/ground_truth.csv --epochs 80 --out results/wacog_e80.pt
python3 wacog/predict.py --ckpt results/wacog_e40.pt ...
python3 wacog/predict.py --ckpt results/wacog_e80.pt ...
python3 analysis/compare.py ...   # compare the two reports
```

### Data volume heuristic (paper-oriented)

| Train graphs with usable regions | Expectation |
|----------------------------------|-------------|
| &lt; 30 | Metrics unstable; do not claim “beats WACO” |
| 100–200 | Reasonable pilot for a workshop / internal report |
| 300+ graphs, thousands of region×layout rows | Closer to a conference-style study |

WACO’s paper used ~2M (matrix, schedule) labels over weeks. You will not match
that; be honest in threats-to-validity (already listed in `comparison_report.md`).

### Hardware calibration note

Always generate `hw_calib.json` on the **same** machine that runs ground truth
and AutoTuner predictions. If you move machines, delete
`~/.config/sgpl/hw_calib.json` and let the harness recreate it.

---

## 6. Outputs to keep for the paper

After a successful full run, archive:

```text
waco_compare/results/
  ground_truth.csv              # measured oracle
  autotuner_pred.csv            # AutoTuner costs + choices
  wacog_pred.csv                # WACO-G scores + choices
  wacog_plans.json              # end-to-end DP plans
  waco_artifact_pred.csv        # Track 2
  comparison_report.md          # human summary
  comparison_report.json        # machine-readable metrics
  wacog.pt                      # trained weights (optional to publish)
```

Key metrics in the report:

- **Agreement** — match rate, Cohen’s κ, confusion
- **vs oracle** — top-1 / top-2, Kendall τ, regret geomean, speedup vs FixedCSR
- **McNemar** — paired significance of who is correct more often
- **Breakdowns** — by family / region kind
- **Threats to validity** — read and keep in the paper

---

## 7. Env vars cheat sheet

| Variable | Role |
|----------|------|
| `FORCE_CPU=1` | Force CPU IR (avoid PTX/GPU compile failures) |
| `GRAPH_DISABLE_POLLY=1` | Skip Polly (not needed for layout study) |
| `AUTOTUNER_FORCE_LAYOUT` | CSR / PCSR / BCSR / SET for GT |
| `AUTOTUNER_PREDICTION_DUMP` | JSONL prediction dump |
| `AUTOTUNER_PROFILE_DUMP` | JSONL measured region times |
| `AUTOTUNER_LAYOUT_PLAN` | `r0:CSR,r1:BCSR,...` for end-to-end plans |
| `OMP_NUM_THREADS` / `SGPL_NUM_THREADS` | Keep equal and fixed for all runs |

---

## 8. Common failures

| Symptom | Fix |
|---------|-----|
| `antlr4-runtime.h` not found | Install antlr4 C++ headers; fix `-I` in `build_lowmem.sh` |
| Undefined Polly symbols at link | Set `POLLY_LIBS="-lPolly -lPollyISL"` or install Polly packs |
| `Support for stacksave requires PTX…` | Export `FORCE_CPU=1` |
| `nlopt.h` / libnlopt missing | Install nlopt or populate `.deps/nlopt` |
| Empty prediction dump | Program has no Traverse/Insert regions; check workload template |
| `answer_mismatch` in GT | Discard those rows (already flagged); investigate nondeterminism |
| CUDA / MinkowskiEngine errors | Ignore — we don’t need them for WACO-G |
| ultralimit / segfault | `ulimit -s unlimited` (harness should set this via run path) |

---

## 9. Branch contents (what landed on `Waco-compare`)

Committed:

- AutoTuner instrumentation (`AutoTunerPass.cpp`, `autotuner_runtime.c`)
- `build_lowmem.sh` include-path fix for Fedora antlr
- Entire `waco_compare/` **harness** (scripts + templates + this handoff)
- `verify/WACO_MAPPING.md` update
- Optional: `paper/WACO_ASPLOS23.pdf`

**Not** committed (regenerate locally):

- `waco_compare/data/` (graphs, builds)
- `waco_compare/third_party/` (clone yourself)
- Large `results/*.pt` / build trees
- `p1GraphEasy-con-AutoTuner/generated/` (run `antlr4 …`)
- `GraphProgram`, `final_program`, `.o` files

---

## 10. Contact / design pointers

- Comparison plan rules: `waco_compare/README.md`
- WACO ↔ AutoTuner mapping: `verify/WACO_MAPPING.md`
- Paper PDF: `paper/WACO_ASPLOS23.pdf` or  
  https://commit.csail.mit.edu/papers/2023/WACO_ASPLOS23.pdf
- Upstream artifact: https://github.com/nullplay/Workload-Aware-Co-Optimization

When in doubt: **freeze the AutoTuner code**, collect ground truth once, then
iterate only on WACO-G epochs / analysis — never retune the AutoTuner against
the test split.
