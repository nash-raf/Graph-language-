#!/usr/bin/env python3
"""Ablation runner for phase-aware autotuner cost model validation.

For each workload, runs 3 layout policies (CSR-only, PCSR-only, DP-optimal),
records DP predictions AND measured profile times, plus wall clock.
"""

import csv
import json
import os
import re
import shutil
import subprocess
import sys
import time as time_mod

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKLOADS_DIR = os.path.join(SCRIPT_DIR, "ablation_workloads")
METADATA = os.path.join(WORKLOADS_DIR, "workloads.csv")
RESULTS_FILE = os.path.join(SCRIPT_DIR, "ablation_results.csv")

# Regexes for parsing
RE_PROFILE_REGION = re.compile(
    r"\[AutoTunerProfile\] region=(\d+) kind=(\S+) layout=(\S+) visits=(\d+) "
    r"predicted_ns=([\d.]+) measured_ns=([\d.]+)"
)
RE_PROFILE_TOTAL = re.compile(
    r"\[AutoTunerProfile\] total kind=(\S+) predicted_ns=([\d.]+) "
    r"measured_ns=([\d.]+) predicted_ms=([\d.]+) measured_ms=([\d.]+) "
    r"pure_kernel_ns=([\d.]+) pure_kernel_ms=([\d.]+)"
)
RE_CONVERSION = re.compile(
    r"\[AutoTunerProfile\] injected (\d+) layout conversions total, conversion_ns=([\d.]+)"
)
RE_ENSURE_LAYOUT = re.compile(
    r"\[autograph_ensure_layout\] graph=(\S+) from=(\S+) to=(\S+)"
)
RE_DP_REGION = re.compile(
    r"\[AutoTuner\]\s+region=(\d+)\s+kind=(\S+)\s+layout=(\S+)\s+"
    r"totalOps=(\d+)\s+execCount=([\d.eE+-]+)\s+"
    r"predicted_ns=([\d.eE+-]+)\s+predicted_ms=([\d.eE+-]+)"
)
RE_FORCED = re.compile(r"\[AutoTuner\] Forced layout override: (\S+)")
RE_TIME_SEC = re.compile(r"TIME_SEC=([\d.]+)")
RE_EST = re.compile(r"estN=([\d.]+)")
RE_INSTR_PAR = re.compile(
    r"Instructions in parallel regions: (\d+)"
)
RE_PRED_TOTAL = re.compile(
    r"\[AutoTuner\]   predicted_op_total_ns=([\d.eE+-]+) "
    r"predicted_total_ns=([\d.eE+-]+) "
    r"predicted_conversion_ns=([\d.eE+-]+)"
)


def tag_from_filename(fname):
    base = os.path.splitext(fname)[0]
    if base.startswith("wl_"):
        base = base[3:]
    return base


def parse_metadata_row(tag):
    m = re.match(r"N(\d+)_D(\d+)_H(\d+)", tag)
    if m:
        return {"N": int(m.group(1)), "D": int(m.group(2)), "H": int(m.group(3))}
    return {"N": None, "D": None, "H": None}


def run_workload(graph_file, tag, force_layout=None, timeout=300):
    """Run full pipeline for one workload+layout, return dict with all data."""

    # Copy edge list
    edgelist_src = os.path.join(WORKLOADS_DIR, f"el_{tag}.txt")
    edgelist_dst = os.path.join(SCRIPT_DIR, "edgelist.txt")
    if not os.path.isfile(edgelist_src):
        return {"error": f"Missing edge list: {edgelist_src}"}
    shutil.copy2(edgelist_src, edgelist_dst)

    # Clean prior build artifacts
    for f in ["program.o", "final_program"]:
        p = os.path.join(SCRIPT_DIR, f)
        if os.path.isfile(p):
            os.remove(p)

    # Build environment
    env = os.environ.copy()
    env["AUTOTUNER_PROFILE_STRICT_REGIONS"] = "1"
    if force_layout:
        env["AUTOTUNER_FORCE_LAYOUT"] = force_layout

    # Phase 1: GraphProgram
    proc = subprocess.run(
        ["./GraphProgram", graph_file],
        cwd=SCRIPT_DIR, env=env,
        capture_output=True, text=True, timeout=timeout,
    )
    if proc.returncode != 0:
        return {"error": f"GraphProgram failed (rc={proc.returncode}): {proc.stderr[:500]}"}

    gp_stderr = proc.stderr
    gp_stdout = proc.stdout

    # Parse DP predictions
    dp_regions = []
    forced = ""
    predicted_op_total_ns = None
    predicted_total_ns = None
    predicted_conversion_ns = None
    for line in gp_stderr.splitlines():
        m = RE_DP_REGION.match(line)
        if m:
            dp_regions.append({
                "region": int(m.group(1)),
                "kind": m.group(2),
                "layout": m.group(3),
                "totalOps": int(m.group(4)),
                "execCount": float(m.group(5)),
                "predicted_ns": float(m.group(6)),
                "predicted_ms": float(m.group(7)),
            })
        m = RE_FORCED.match(line)
        if m:
            forced = m.group(1)
        m = RE_PRED_TOTAL.match(line)
        if m:
            predicted_op_total_ns = float(m.group(1))
            predicted_total_ns = float(m.group(2))
            predicted_conversion_ns = float(m.group(3))

    # Parse graph size
    est_n = None
    for line in gp_stderr.splitlines():
        m = RE_EST.search(line)
        if m:
            est_n = float(m.group(1))
            break

    # Phase 2: Build runtime .o files
    build_cmds = [
        "gcc -O3 -c autotuner_runtime.c -o autotuner_runtime.o",
        "gcc -O3 -c graph_mutation_runtime.c -o graph_mutation_runtime.o",
        "gcc -O3 -c parallel_runtime.c -o parallel_runtime.o",
        "gcc -O3 -c runtime.c -o runtime.o",
        "g++ -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o",
        "g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o",
        "g++ -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o",
    ]

    for cmd in build_cmds:
        r = subprocess.run(cmd, cwd=SCRIPT_DIR, shell=True,
                           capture_output=True, text=True, timeout=60)
        if r.returncode != 0:
            return {"error": f"Build failed: {cmd}\n{r.stderr[:500]}"}

    # NLOpt link flags
    nlopt_prefix = os.environ.get("SGPL_NLOPT_PREFIX",
                                  os.path.join(SCRIPT_DIR, ".deps/nlopt"))
    if os.path.isfile(os.path.join(nlopt_prefix, "include/nlopt.h")):
        libdir = (os.path.join(nlopt_prefix, "lib64")
                  if os.path.isdir(os.path.join(nlopt_prefix, "lib64"))
                  else os.path.join(nlopt_prefix, "lib"))
        ld_lib = env.get("LD_LIBRARY_PATH", "")
        env["LD_LIBRARY_PATH"] = f"{libdir}:{ld_lib}" if ld_lib else libdir
        nlopt_flags = f"-L{libdir} -Wl,-rpath,{libdir} -lnlopt"
    else:
        nlopt_flags = "-lnlopt"

    link_cmd = (
        f"g++ -O3 -fopenmp -no-pie "
        f"program.o runtime.o parallel_runtime.o autotuner_runtime.o "
        f"graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o "
        f"graph_runtime.o {nlopt_flags} -o final_program"
    )
    r = subprocess.run(link_cmd, cwd=SCRIPT_DIR, shell=True,
                       capture_output=True, text=True, timeout=60)
    if r.returncode != 0:
        return {"error": f"Link failed: {link_cmd}\n{r.stderr[:500]}"}

    # Phase 3: Run final_program with timing
    env["OMP_NUM_THREADS"] = env.get("OMP_NUM_THREADS", str(os.cpu_count()))
    env["SGPL_ROARING_THREADS"] = env.get("SGPL_ROARING_THREADS", "4")

    time_proc = subprocess.run(
        ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", "./final_program"],
        cwd=SCRIPT_DIR, env=env,
        capture_output=True, text=True, timeout=timeout,
    )

    # Parse wall time
    actual_sec = None
    for line in time_proc.stderr.splitlines():
        m = RE_TIME_SEC.match(line)
        if m:
            actual_sec = float(m.group(1))
            break

    combined_output = time_proc.stderr + "\n" + time_proc.stdout

    # Parse profile output
    profile_regions = []
    profile_totals = {}
    total_conversions = None
    total_conversion_ns = None
    ensure_layout_events = []

    for line in combined_output.splitlines():
        m = RE_PROFILE_REGION.match(line)
        if m:
            profile_regions.append({
                "region": int(m.group(1)),
                "kind": m.group(2),
                "layout": m.group(3),
                "visits": int(m.group(4)),
                "predicted_ns": float(m.group(5)),
                "measured_ns": float(m.group(6)),
            })
        m = RE_PROFILE_TOTAL.match(line)
        if m:
            profile_totals[m.group(1)] = {
                "predicted_ns": float(m.group(2)),
                "measured_ns": float(m.group(3)),
                "pure_kernel_ns": float(m.group(6)),
                "pure_kernel_ms": float(m.group(7)),
            }
        m = RE_CONVERSION.match(line)
        if m:
            total_conversions = int(m.group(1))
            total_conversion_ns = float(m.group(2))
        m = RE_ENSURE_LAYOUT.match(line)
        if m:
            ensure_layout_events.append({
                "graph": m.group(1),
                "from": m.group(2),
                "to": m.group(3),
            })

    return {
        "dp_regions": dp_regions,
        "forced_layout": forced,
        "predicted_op_total_ns": predicted_op_total_ns,
        "predicted_total_ns": predicted_total_ns,
        "predicted_conversion_ns": predicted_conversion_ns,
        "est_n": est_n,
        "profile_regions": profile_regions,
        "profile_totals": profile_totals,
        "total_conversions": total_conversions,
        "total_conversion_ns": total_conversion_ns,
        "ensure_layout_events": json.dumps(ensure_layout_events),
        "actual_sec": actual_sec,
    }


def main():
    # Read workload metadata
    workloads = []
    with open(METADATA) as f:
        reader = csv.DictReader(f)
        wl_list = list(reader)

    # Limit to a subset for testing — comment out for full run
    # wl_list = [w for w in wl_list if w["workload"] == "N50000_D2_H10"]

    csv_rows = []

    for wl in wl_list:
        tag = wl["workload"]
        graph_file = os.path.join(WORKLOADS_DIR, wl["graph_file"])
        if not os.path.isfile(graph_file):
            print(f"  SKIP {graph_file} (not found)")
            continue

        print(f"\n{'='*60}")
        print(f"Workload: {tag}")
        print(f"{'='*60}")

        for layout_name, force_val in [("CSR", "CSR"), ("PCSR", "PCSR"), ("DP", None)]:
            print(f"  [{layout_name}] Running...", flush=True)
            t0 = time_mod.time()
            result = run_workload(graph_file, tag, force_layout=force_val)
            elapsed = time_mod.time() - t0

            if "error" in result:
                print(f"  [{layout_name}] ERROR: {result['error']}")
                csv_rows.append({
                    **parse_metadata_row(tag),
                    "tag": tag, "layout": layout_name,
                    "error": result["error"],
                })
                continue

            actual_ms = result["actual_sec"] * 1000.0 if result["actual_sec"] is not None else None

            # DP predictions
            dp_total_ns = sum(r["predicted_ns"] for r in result["dp_regions"])
            dp_region_info = json.dumps(result["dp_regions"])

            # Profile measured totals
            trav = result["profile_totals"].get("Traverse", {})
            ins = result["profile_totals"].get("Insert", {})
            query = result["profile_totals"].get("Query", {})

            pure_kernel_total_ns = (
                trav.get("pure_kernel_ns", 0) +
                ins.get("pure_kernel_ns", 0) +
                query.get("pure_kernel_ns", 0)
            )

            row = {
                **parse_metadata_row(tag),
                "tag": tag,
                "layout": layout_name,
                "actual_ms": actual_ms,
                "dp_predicted_total_ns": dp_total_ns,
                "dp_predicted_total_ms": dp_total_ns / 1.0e6,
                "dp_forced": result["forced_layout"],
                "num_dp_regions": len(result["dp_regions"]),
                "dp_regions": dp_region_info,
                "profile_regions": json.dumps(result["profile_regions"]),
                "predicted_op_total_ns": result["predicted_op_total_ns"],
                "predicted_total_ns": result["predicted_total_ns"],
                "predicted_conversion_ns": result["predicted_conversion_ns"],
                "measured_traverse_ms": trav.get("measured_ms"),
                "measured_insert_ms": ins.get("measured_ms"),
                "pure_kernel_traverse_ms": trav.get("pure_kernel_ms"),
                "pure_kernel_insert_ms": ins.get("pure_kernel_ms"),
                "pure_kernel_total_ms": pure_kernel_total_ns / 1.0e6,
                "total_conversions": result["total_conversions"],
                "total_conversion_ns": result["total_conversion_ns"],
                "ensure_layout_events": result["ensure_layout_events"],
                "est_n": result["est_n"],
            }
            csv_rows.append(row)
            print(f"  [{layout_name}] Done in {elapsed:.0f}s | "
                  f"actual={actual_ms:.1f}ms kernel={pure_kernel_total_ns/1.0e6:.1f}ms "
                  f"dp_pred={dp_total_ns/1.0e6:.2f}ms conv={result['total_conversion_ns'] or 0:.0f}ns")

    # Write CSV
    if csv_rows:
        fieldnames = [
            "tag", "N", "D", "H",
            "layout", "actual_ms",
            "dp_predicted_total_ns", "dp_predicted_total_ms",
            "dp_forced", "num_dp_regions", "dp_regions", "profile_regions",
            "predicted_op_total_ns", "predicted_total_ns", "predicted_conversion_ns",
            "measured_traverse_ms", "measured_insert_ms",
            "pure_kernel_traverse_ms", "pure_kernel_insert_ms",
            "pure_kernel_total_ms",
            "total_conversions", "total_conversion_ns",
            "ensure_layout_events",
            "est_n", "error",
        ]
        with open(RESULTS_FILE, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
            w.writeheader()
            w.writerows(csv_rows)
        print(f"\nResults written to {RESULTS_FILE} ({len(csv_rows)} rows)")
    else:
        print("\nNo results to write.")


if __name__ == "__main__":
    main()
