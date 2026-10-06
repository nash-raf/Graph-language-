#!/usr/bin/env python3
"""Universal thread-budget validation: enumerate every allocation between two
task loops of one TDG level and compare against the model's own choice.

Model under test (parallel_runtime.c, sgpl_run_tdg_level):
    level_budget   -- threads available to the level
    chosen_threads -- the level's own width (one per task)
    loop_budget    -- level_budget - chosen_threads
    the level plan then distributes loop_budget across the level's task loops,
    minimising the modelled total time (sgpl_choose_tdg_threads_with_loop_budget).

What this does:
    1. builds budget_alloc_test.c (two registered loop sites: 101 light, 102 heavy,
       real work, repeated rounds, 2-task level),
    2. runs it once unforced and reads [tdg.plan] -> the model's chosen allocation,
    3. runs it once per split w1 + w2 == loop_budget with SGPL_FORCE_WIDTHS pinning
       the split, best of --reps wall times,
    4. writes CSV + markdown table + matplotlib figure (bar plot, chosen split
       marked) into --outdir.

Usage: budget_alloc_sweep.py [--threads 16] [--reps 3] [--outdir DIR]
"""
import argparse
import csv
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "p1GraphEasy-con-AutoTuner"))

PLAN_RE = re.compile(
    r"\[tdg\.plan\] level entries=(\d+) budget=(\d+) loop_budget=(\d+) idle=(\d+) "
    r"dominant=(-?\d+) total_model_ns=([0-9.]+)")
ENTRY_RE = re.compile(
    r"\[tdg\.plan\]\s+slot=(\d+) loop_id=(-?\d+) assigned_threads=(\d+) "
    r"c_rank_ns=([0-9.]+) model_ns=([0-9.]+)")
RESULT_RE = re.compile(
    r"RESULT rounds=(\d+) trips=(\d+) ratio=(\d+) threads=(\d+) forced=(\S+) "
    r"time_ns=(\d+) light_workers=(\d+) light_trips=(\d+) heavy_workers=(\d+) "
    r"heavy_trips=(\d+)(?: last_light_ns=(\d+) last_heavy_ns=(\d+))?")


def build(binpath):
    nlopt = os.path.join(REPO, ".deps", "nlopt")
    inc = ["-I" + os.path.join(nlopt, "include")] if os.path.isdir(nlopt) else []
    lib = (["-L" + os.path.join(nlopt, "lib"), "-L" + os.path.join(nlopt, "lib64"),
            "-Wl,-rpath," + os.path.join(nlopt, "lib"),
            "-Wl,-rpath," + os.path.join(nlopt, "lib64")]
           if os.path.isdir(nlopt) else [])
    objs = []
    for src, cc in (("budget_alloc_test.c", "gcc"), ("parallel_runtime.c", "gcc"),
                    ("gpu_runtime.c", "gcc"), ("roaring_bitmap.cpp", "g++")):
        obj = binpath + "." + os.path.basename(src) + ".o"
        arch = ["-mavx2", "-march=native"] if src.endswith(".cpp") else []
        cmd = [cc, "-O3", "-fopenmp"] + arch + inc + \
              ["-c", os.path.join(REPO, src), "-o", obj]
        subprocess.run(cmd, check=True)
        objs.append(obj)
    subprocess.run(["g++", "-O3", "-fopenmp"] + objs + ["-o", binpath] + lib +
                   ["-lnlopt", "-lpthread", "-lm", "-ldl"], check=True)
    for obj in objs:
        os.unlink(obj)
    return binpath


def run_once(binpath, threads, rounds, trips, ratio, force=None, timeout=900):
    env = dict(os.environ)
    env["SGPL_NUM_THREADS"] = str(threads)
    env["OMP_NUM_THREADS"] = str(threads)
    env["SGPL_ALLOC_ROUNDS"] = str(rounds)
    env["SGPL_ALLOC_TRIPS"] = str(trips)
    env["SGPL_ALLOC_RATIO"] = str(ratio)
    env["SGPL_BUDGET_DEBUG"] = "1"
    env["SGPL_TDG_PLAN_DUMP"] = "1"
    if force:
        env["SGPL_FORCE_WIDTHS"] = force
    else:
        env.pop("SGPL_FORCE_WIDTHS", None)
    p = subprocess.run([binpath], env=env, capture_output=True, text=True,
                       timeout=timeout)
    res = {"stdout": p.stdout, "stderr": p.stderr}
    m = RESULT_RE.search(p.stdout)
    if not m:
        raise RuntimeError("no RESULT line:\n" + p.stdout + p.stderr[-2000:])
    (res["rounds"], res["trips"], res["ratio"], res["threads"], res["forced"],
     res["time_ns"], res["light_workers"], res["light_trips"],
     res["heavy_workers"], res["heavy_trips"]) = (
        int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)),
        m.group(5), int(m.group(6)), int(m.group(7)), int(m.group(8)),
        int(m.group(9)), int(m.group(10)))
    res["last_light_ns"] = int(m.group(11)) if m.group(11) else -1
    res["last_heavy_ns"] = int(m.group(12)) if m.group(12) else -1
    plans = []
    for pm in PLAN_RE.finditer(p.stderr):
        block = {"entries": int(pm.group(1)), "budget": int(pm.group(2)),
                 "loop_budget": int(pm.group(3)), "idle": int(pm.group(4)),
                 "dominant": int(pm.group(5)), "total_model_ns": float(pm.group(6))}
        start = pm.end()
        entries = []
        for em in ENTRY_RE.finditer(p.stderr, start, start + 400):
            entries.append({"slot": int(em.group(1)), "loop_id": int(em.group(2)),
                            "assigned_threads": int(em.group(3)),
                            "c_rank_ns": float(em.group(4)),
                            "model_ns": float(em.group(5))})
        block["entries"] = entries
        plans.append(block)
    res["plans"] = plans
    # The pool path is the authoritative allocation record: it is printed for
    # every registered loop site of the level, whatever path built the plan.
    alloc = {}
    for am in re.finditer(r"\[budget\.loop-pool\.reserve\] task_slot=(\d+) "
                          r"loop_id=(-?\d+) requested=(\d+) granted=(\d+) "
                          r"remaining=(-?\d+) budget=(\d+)", p.stderr):
        alloc[int(am.group(2))] = {"task_slot": int(am.group(1)),
                                   "requested": int(am.group(3)),
                                   "granted": int(am.group(4)),
                                   "remaining": int(am.group(5)),
                                   "budget": int(am.group(6))}
    res["alloc"] = alloc
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--threads", type=int, default=16)
    ap.add_argument("--reps", type=int, default=3)
    ap.add_argument("--rounds", type=int, default=12)
    ap.add_argument("--trips", type=int, default=1 << 20)
    ap.add_argument("--ratio", type=int, default=3)
    ap.add_argument("--bin", default="/tmp/budget_alloc_test")
    ap.add_argument("--outdir", default=HERE)
    args = ap.parse_args()

    print(f"building {args.bin} ...", flush=True)
    build(args.bin)

    print("model run (unforced) ...", flush=True)
    model = run_once(args.bin, args.threads, args.rounds, args.trips, args.ratio)
    plan = model["plans"][-1] if model["plans"] else None
    if not plan:
        print("no level plan observed at all; is the level runner reached?",
              file=sys.stderr)
        print("\n".join(l for l in model["stderr"].splitlines()
                         if l.startswith("[tdg") or l.startswith("[budget"))[:2000],
              file=sys.stderr)
        return 1
    if not plan["entries"]:
        print("level plan has no loop entries (serial decision): using the pool "
              "allocation record for the model's choice", file=sys.stderr)
    by_id = {e["loop_id"]: e for e in plan["entries"]}
    model_w1 = by_id.get(101, {}).get("assigned_threads", -1)
    model_w2 = by_id.get(102, {}).get("assigned_threads", -1)
    model_alloc = model.get("alloc", {})
    if (model_w1 < 0 or model_w2 < 0) and model_alloc:
        model_w1 = model_alloc.get(101, {}).get("granted", model_w1)
        model_w2 = model_alloc.get(102, {}).get("granted", model_w2)
    alloc0 = model.get("alloc", {})
    loop_budget = plan["loop_budget"] or alloc0.get(101, {}).get("budget", 0)
    level_budget = plan["budget"]
    chosen = level_budget - loop_budget if loop_budget else 0
    print(f"model chose: light(101)={model_w1} heavy(102)={model_w2} "
          f"loop_budget={loop_budget} level_budget={level_budget} "
          f"level_width={chosen} plan_entries={len(plan['entries'])}", flush=True)

    rows = []
    for w1 in range(1, loop_budget):
        w2 = loop_budget - w1
        best = None
        for _ in range(args.reps):
            r = run_once(args.bin, args.threads, args.rounds, args.trips, args.ratio,
                         force=f"101:{w1},102:{w2}")
            if best is None or r["time_ns"] < best["time_ns"]:
                best = r
        alloc = best.get("alloc", {})
        rows.append({"w1_light": w1, "w2_heavy": w2,
                     "light_granted": alloc.get(101, {}).get("granted", -1),
                     "heavy_granted": alloc.get(102, {}).get("granted", -1),
                     "pool_budget": alloc.get(101, {}).get("budget", -1),
                     "time_ns": best["time_ns"],
                     "time_per_round_ns": best["time_ns"] / best["rounds"],
                     "light_workers": best["light_workers"],
                     "heavy_workers": best["heavy_workers"],
                     "last_light_ns": best["last_light_ns"],
                     "last_heavy_ns": best["last_heavy_ns"],
                     "task_sum_ns": best["last_light_ns"] + best["last_heavy_ns"]
                     if best["last_light_ns"] >= 0 else -1,
                     "is_model_choice": 1 if (w1 == model_w1 and w2 == model_w2) else 0})
        print(f"  w1={w1:2d} w2={w2:2d} time={best['time_ns']/1e6:8.2f} ms "
              f"(light_w={best['light_workers']:2d} heavy_w={best['heavy_workers']:2d})",
              flush=True)

    os.makedirs(args.outdir, exist_ok=True)
    csv_path = os.path.join(args.outdir, "budget_alloc.csv")
    with open(csv_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)

    best_row = min(rows, key=lambda r: r["time_ns"])
    mdl = next((r for r in rows if r["is_model_choice"]), None)
    md_path = os.path.join(args.outdir, "budget_alloc_table.md")
    with open(md_path, "w") as f:
        f.write("# Universal thread-budget allocation sweep\n\n")
        f.write(f"- level budget: {level_budget} threads, level width {chosen} "
                f"(plan entries {len(plan['entries'])}), loop budget {loop_budget} "
                f"split between two loops\n")
        f.write(f"- threads={args.threads} rounds={args.rounds} trips={args.trips} "
                f"ratio(heavy:light)={args.ratio} reps={args.reps} (best of)\n")
        f.write(f"- model chose light={model_w1} heavy={model_w2}; "
                f"measured optimum light={best_row['w1_light']} "
                f"heavy={best_row['w2_heavy']} "
                f"({best_row['time_ns']/1e6:.2f} ms)\n\n")
        f.write("| light(101) | heavy(102) | time (ms) | ms/round | light workers "
                "| heavy workers | note |\n")
        f.write("|---|---|---|---|---|---|---|\n")
        for r in rows:
            note = []
            if r["is_model_choice"]:
                note.append("**model choice**")
            if r is best_row:
                note.append("measured optimum")
            f.write(f"| {r['w1_light']} | {r['w2_heavy']} | "
                    f"{r['time_ns']/1e6:.2f} | {r['time_per_round_ns']/1e6:.3f} | "
                    f"{r['light_workers']} | {r['heavy_workers']} | "
                    f"{', '.join(note)} |\n")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt

        labels = [f"{r['w1_light']}+{r['w2_heavy']}" for r in rows]
        times = [r["time_ns"] / 1e6 for r in rows]
        opt_idx = min(range(len(rows)), key=lambda i: rows[i]["time_ns"])
        mdl_idx = rows.index(mdl) if mdl else -1
        colors = ["#c8c8c8"] * len(rows)
        if mdl_idx >= 0:
            colors[mdl_idx] = "#1f77b4"
        colors[opt_idx] = "#2ca02c" if opt_idx == mdl_idx else "#d62728"

        fig, ax = plt.subplots(figsize=(max(9.0, 0.55 * len(rows) + 4.0), 5.8))
        ax.bar(range(len(rows)), times, color=colors)
        if mdl_idx >= 0:
            ax.annotate(f"model choice {rows[mdl_idx]['w1_light']}+{rows[mdl_idx]['w2_heavy']}",
                        (mdl_idx, times[mdl_idx]), textcoords="offset points",
                        xytext=(0, 7), ha="center", color="#1f77b4", fontsize=9,
                        fontweight="bold")
        ax.annotate(f"measured optimum {rows[opt_idx]['w1_light']}+{rows[opt_idx]['w2_heavy']}",
                    (opt_idx, times[opt_idx]), textcoords="offset points",
                    xytext=(0, 7), ha="center", color="#2ca02c" if opt_idx == mdl_idx else "#d62728",
                    fontsize=9, fontweight="bold")
        ax.set_xticks(range(len(rows)))
        ax.set_xticklabels(labels, rotation=90, fontsize=8)
        ax.set_xlabel("allocation  light threads + heavy threads")
        ax.set_ylabel(f"level wall time (ms), best of {args.reps}")
        verdict = "model choice == measured optimum" if mdl_idx == opt_idx else \
                  "model choice != measured optimum"
        ax.set_title("Universal thread-budget model: every allocation vs the model's choice\n"
                     f"2 task loops, level budget {level_budget}, level width {chosen}, "
                     f"loop budget {loop_budget}, round {args.rounds}, trips {args.trips} "
                     f"({verdict})")
        ax.grid(True, axis="y", alpha=0.3)
        from matplotlib.patches import Patch
        handles = [Patch(color="#1f77b4", label="allocation the model chose"),
                   Patch(color="#2ca02c" if opt_idx == mdl_idx else "#d62728",
                         label="measured optimum"),
                   Patch(color="#c8c8c8", label="other allocations")]
        ax.legend(handles=handles, loc="upper right", fontsize=9)
        fig.tight_layout()
        png = os.path.join(args.outdir, "budget_alloc.png")
        fig.savefig(png, dpi=140)
        fig.savefig(os.path.join(args.outdir, "budget_alloc.svg"))
        print(f"wrote {png}")
    except ImportError:
        print("matplotlib not available: CSV + markdown table written only",
              file=sys.stderr)

    print(f"wrote {csv_path}")
    print(f"wrote {md_path}")
    print(f"model choice: light={model_w1} heavy={model_w2} | "
          f"measured optimum: light={best_row['w1_light']} heavy={best_row['w2_heavy']} | "
          f"{'MATCH' if mdl and mdl is best_row else 'MISMATCH'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
