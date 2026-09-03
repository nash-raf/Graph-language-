"""insitu_probe.py — Measure the REAL memmove inside autograph_bcsr_add_edge
on the real graphs (in-situ t_D), using the env-guarded profiling added to
autotuner_runtime.c (AUTOTUNER_SHIFT_PROFILE).

For each graph: build the standard 50-pair insert workload (bench_folder),
run it under layout=BCSR (bench_folder.compile_and_run), then read the
profile line the runtime appended at exit:

  {shift_ns, shift_cnt, shift_bytes, shift_per_line, reloc_cnt, reloc_bytes}

Prints per graph: in-situ shift per-line rate, avg move bytes, realloc
migration count/bytes, the measured cell (ns/op), and the cost model's
cMove prediction for the same avg move size.

Usage:  AUTOTUNER_FORCE_LAYOUT=BCSR  (set inside)  python3 insitu_probe.py
"""
import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bench_folder
import cost_model as cm

GRAPHS = [
    "fb-pages-company",
    "road-usroads-48",
    "road-luxembourg-osm",
    "bio-grid-yeast",
    "ia-dbpedia",
    "copresence-InVS15",
    "inf-roadNet-PA",
    "road-roadNet-PA",
    "soc-LiveMocha",
]

K = 50


def main():
    out_dir = os.path.join(tempfile.gettempdir(), "insitu_profiles")
    os.makedirs(out_dir, exist_ok=True)
    wdir = tempfile.mkdtemp(prefix="insitu_probe_")
    os.environ["AUTOTUNER_FORCE_LAYOUT"] = "BCSR"

    print(f"{'graph':24s} {'m':>8s} {'bcolMB':>7s} | "
          f"{'shift/line':>10s} {'avgMoveB':>9s} {'relocs':>7s} {'relocMB':>8s} | "
          f"{'meas/op':>10s} {'predMove':>10s}")
    for g in GRAPHS:
        ef = cm._resolve_edge_file(g)
        if not ef:
            print(f"{g:24s} MISSING EDGE FILE")
            continue
        nv, m_und = bench_folder.count_graph_file(ef)
        insert_graph = os.path.join(wdir, f"{g}_insert.graph")
        try:
            n_fresh = bench_folder.make_insert_workload(
                ef, K, nv, __import__("pathlib").Path(insert_graph))
        except Exception as e:
            print(f"{g:24s} workload FAILED: {e}")
            continue

        profile = os.path.join(out_dir, f"{g}.jsonl")
        os.environ["AUTOTUNER_SHIFT_PROFILE"] = profile
        if os.path.exists(profile):
            os.unlink(profile)
        res = bench_folder.compile_and_run(insert_graph, "BCSR", wdir, 600, 7)
        if res.get("error"):
            print(f"{g:24s} RUN FAILED: {res['error'][:60]}")
            continue

        # Last profile line = one of the measured processes (all identical
        # workloads; the runtime appends one JSON line per process at exit).
        lines = []
        if os.path.exists(profile):
            with open(profile) as f:
                lines = [l.strip() for l in f if l.strip()]
        if not lines:
            print(f"{g:24s} NO PROFILE (no BCSR adds ran?)")
            continue
        p = json.loads(lines[-1])

        # Measured cell from the CSVs, if present.
        meas = None
        for path in ("real_world_runs10_merged.csv",
                     "real_world_runs10.csv", "meh_validate_updated.csv"):
            if not os.path.exists(path):
                continue
            import csv
            with open(path) as f:
                for r in csv.DictReader(f):
                    if ((r.get("op") or r.get("operation")) == "Insert"
                            and (r.get("graph") or r.get("graph_label")) == g
                            and r.get("layout") == "BCSR"):
                        try:
                            meas = float(r.get("measured_ns")
                                         or r.get("measured_kernel_ns")) / K
                        except (TypeError, ValueError):
                            meas = None
                        break
            if meas:
                break

        avg_move = p["shift_bytes"] / max(p["shift_cnt"], 1)
        pred_move = cm._curve_per_line(avg_move)  # model ns/line at same size
        print(f"{g:24s} {m_und:8d} {2*m_und*4/2**20:7.2f} | "
              f"{p['shift_per_line']:10.3f} {avg_move:9.0f} "
              f"{p['reloc_cnt']:7d} {p['reloc_bytes']/2**20:8.2f} | "
              f"{meas if meas else 0:10.0f} {pred_move:10.3f}")

    print(f"\nprofiles: {out_dir}")


if __name__ == "__main__":
    main()