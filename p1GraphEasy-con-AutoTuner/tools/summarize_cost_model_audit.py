#!/usr/bin/env python3
"""Summarize independent dispatch/service measurements without fitting constants."""
import csv
import json
import pathlib
import statistics
import sys

from cost_model_equations import Site, choose_budget, level_ns


def errors(predictions, actuals):
    values = [abs(p / a - 1) for p, a in zip(predictions, actuals)]
    return {"count": len(values), "median_ape_percent": statistics.median(values) * 100,
            "mean_ape_percent": statistics.mean(values) * 100,
            "within_20_percent": sum(v <= .2 for v in values),
            "within_30_percent": sum(v <= .3 for v in values)}


def main(directory):
    directory = pathlib.Path(directory)
    records = list(csv.reader(line for line in (directory / "raw.csv").read_text().splitlines()
                              if line and not line.startswith("#")))
    all_rows = [list(map(float, row[1:])) for row in records if row[0] == "all"]
    pairs = [list(map(float, row[1:])) for row in records if row[0] == "pair"]
    across = [list(map(float, row[1:])) for row in records if row[0] == "across"]
    across_proposal = [list(map(float, row[1:])) for row in records if row[0] == "across-proposal"]
    old = [1200 + 180 * r[0] + r[4] / r[0] for r in all_rows]
    summary = {
        "doall_old_equation_on_same_inputs": errors(old, [r[6] for r in all_rows]),
        "doall_corrected": errors([r[5] for r in all_rows], [r[6] for r in all_rows]),
        "doacross_launch_old": errors([2700 + 180 * r[0] for r in across], [r[7] for r in across]),
        "doacross_launch_corrected": errors([r[6] for r in across], [r[7] for r in across]),
        "serial_profile_interval_fraction": statistics.median(r[3] / r[4] for r in across),
        "pool_launch_measured_ns_range": [min(r[8] for r in all_rows), max(r[8] for r in all_rows)],
        "doacross_mechanism_proposal": errors([r[3] for r in across_proposal], [r[4] for r in across_proposal]),
        "budget_composition_all_pairs": {
            "old_sum": errors([r[9] for r in pairs], [r[11] for r in pairs]),
            "resource": errors([r[10] for r in pairs], [r[11] for r in pairs])},
    }
    work_pairs = [r for r in pairs if r[11] >= 1e6]
    summary["budget_composition_pairs_above_1ms"] = {
        "old_sum": errors([r[9] for r in work_pairs], [r[11] for r in work_pairs]),
        "resource": errors([r[10] for r in work_pairs], [r[11] for r in work_pairs])}
    selections = []
    for case in sorted(set(tuple(r[:5]) for r in pairs)):
        rs = [r for r in pairs if tuple(r[:5]) == case]
        curves = [{}, {}]
        for row in rs:
            curves[0].setdefault(int(row[5]), []).append(row[7])
            curves[1].setdefault(int(row[6]), []).append(row[8])
        sites = [Site(i, {w: statistics.median(v) for w, v in curve.items()})
                 for i, curve in enumerate(curves)]
        # Held-out pair timings never enter the prediction or selection.
        selected = min(rs, key=lambda r: level_ns(sites, tuple(map(int, r[5:7])), 2))
        optimum = min(rs, key=lambda r: r[11])
        policy = choose_budget(sites, int(case[0]) + 2, 2)
        selections.append({"P_N_chain_streaming_ratio": case,
                           "chosen_among_measured_pairs": selected[5:7],
                           "measured_best_pair": optimum[5:7],
                           "regret_percent": (selected[11] / optimum[11] - 1) * 100,
                           "reference_search": policy})
    summary["budget_holdout_selections"] = selections
    summary["budget_selection_median_regret_percent"] = statistics.median(r["regret_percent"] for r in selections)
    summary["budget_selection_max_regret_percent"] = max(r["regret_percent"] for r in selections)
    # Identical recurrence body; only distance differs. No equation fitting.
    summary["doacross_distance_contrast"] = [
        {"P": p, "d1_parallel_ns": next(r[5] for r in across if r[0] == p and r[1] == 200000 and r[2] == 1),
         "d4_parallel_ns": next(r[5] for r in across if r[0] == p and r[1] == 200000 and r[2] == 4)}
        for p in (2, 4, 8)]
    (directory / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main(sys.argv[1])
