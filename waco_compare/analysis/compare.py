#!/usr/bin/env python3
"""Paper metrics: agreement, accuracy vs oracle, regret, significance."""
from __future__ import annotations

import argparse
import csv
import json
import math
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
LAYOUTS = ["CSR", "PCSR", "BCSR", "SET"]


def load_csv(path: Path) -> list[dict]:
    if not path.is_file():
        return []
    return list(csv.DictReader(path.open()))


def oracle_from_gt(gt_rows):
    """Per (graph, template, region) measured times and best set (within 5% / CI)."""
    groups = defaultdict(dict)
    meta = {}
    for r in gt_rows:
        if r.get("error") or r.get("kind") not in ("Traverse", "Insert"):
            continue
        key = (r["graph"], r["template"], int(float(r["region_id"])))
        groups[key][r["layout"]] = float(r["measured_ns_median"])
        meta[key] = r
    oracle = {}
    for key, d in groups.items():
        if len(d) < 2:
            continue
        best = min(d.values())
        # within 5% counts as best
        best_set = {L for L, t in d.items() if t <= best * 1.05}
        oracle[key] = {"times": d, "best": min(d, key=d.get), "best_set": best_set}
    return oracle


def kendall_tau(pred_order, true_times):
    """Kendall tau-b between predicted ranking and measured times (lower better)."""
    layouts = [L for L in LAYOUTS if L in true_times and L in pred_order]
    n = len(layouts)
    if n < 2:
        return float("nan")
    # ranks: lower score/time => better (rank 0)
    pred_rank = {L: i for i, L in enumerate(sorted(layouts, key=lambda x: pred_order[x]))}
    true_rank = {L: i for i, L in enumerate(sorted(layouts, key=lambda x: true_times[x]))}
    conc = disc = 0
    for i in range(n):
        for j in range(i + 1, n):
            a, b = layouts[i], layouts[j]
            p = np.sign(pred_rank[a] - pred_rank[b])
            t = np.sign(true_rank[a] - true_rank[b])
            if p == t:
                conc += 1
            else:
                disc += 1
    denom = conc + disc
    return (conc - disc) / denom if denom else float("nan")


def mcnemar(correct_a, correct_b):
    """McNemar exact-ish statistic on paired boolean correctness."""
    n01 = n10 = 0
    for a, b in zip(correct_a, correct_b):
        if (not a) and b:
            n01 += 1
        elif a and (not b):
            n10 += 1
    n = n01 + n10
    if n == 0:
        return {"n01": n01, "n10": n10, "p_approx": 1.0}
    # chi-square with continuity correction
    chi = (abs(n01 - n10) - 1) ** 2 / n
    # rough p via erfc for chi~1df
    p = math.erfc(math.sqrt(chi / 2.0))
    return {"n01": n01, "n10": n10, "chi2": chi, "p_approx": p}


def bootstrap_ci(values, n_boot=1000, alpha=0.05, seed=0):
    rng = np.random.default_rng(seed)
    values = np.asarray(values, dtype=float)
    if len(values) == 0:
        return (float("nan"), float("nan"), float("nan"))
    means = []
    for _ in range(n_boot):
        sample = rng.choice(values, size=len(values), replace=True)
        means.append(sample.mean())
    lo, hi = np.quantile(means, [alpha / 2, 1 - alpha / 2])
    return float(values.mean()), float(lo), float(hi)


def cohen_kappa(y1, y2, labels):
    n = len(y1)
    if n == 0:
        return float("nan")
    agree = sum(a == b for a, b in zip(y1, y2)) / n
    c1 = Counter(y1)
    c2 = Counter(y2)
    pe = sum((c1[L] / n) * (c2[L] / n) for L in labels)
    if pe >= 1:
        return 1.0
    return (agree - pe) / (1 - pe)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gt", type=Path, default=ROOT / "results" / "ground_truth.csv")
    ap.add_argument("--at", type=Path, default=ROOT / "results" / "autotuner_pred.csv")
    ap.add_argument("--wacog", type=Path, default=ROOT / "results" / "wacog_pred.csv")
    ap.add_argument("--art", type=Path, default=ROOT / "results" / "waco_artifact_pred.csv")
    ap.add_argument("--out", type=Path, default=ROOT / "results" / "comparison_report.json")
    ap.add_argument("--md", type=Path, default=ROOT / "results" / "comparison_report.md")
    args = ap.parse_args()

    gt = load_csv(args.gt)
    at = load_csv(args.at)
    wg = load_csv(args.wacog)
    art = load_csv(args.art)
    oracle = oracle_from_gt(gt)

    at_by = {
        (r["graph"], r["template"], int(float(r["region_id"]))): r
        for r in at
        if r.get("dominant") in ("Traverse", "Insert") and not r.get("error")
    }
    wg_by = {
        (r["graph"], r["template"], int(float(r["region_id"]))): r for r in wg
    }

    keys = sorted(set(oracle) & set(at_by) & set(wg_by))
    report = {"n_regions": len(keys), "layouts": LAYOUTS}

    # Agreement
    at_preds, wg_preds = [], []
    conf = Counter()
    for k in keys:
        a = at_by[k].get("local_argmin") or at_by[k].get("dp_chosen")
        w = wg_by[k]["local_argmin"]
        at_preds.append(a)
        wg_preds.append(w)
        conf[(a, w)] += 1
    match = sum(a == w for a, w in zip(at_preds, wg_preds)) / max(len(keys), 1)
    report["agreement"] = {
        "match_rate": match,
        "cohen_kappa": cohen_kappa(at_preds, wg_preds, LAYOUTS),
        "confusion": {f"{a}->{b}": v for (a, b), v in conf.items()},
    }

    # Accuracy vs oracle
    def eval_tool(pred_fn):
        top1 = []
        top2 = []
        regret = []
        taus = []
        correct = []
        speedup_vs_csr = []
        for k in keys:
            o = oracle[k]
            pred = pred_fn(k)
            if pred is None:
                continue
            ok = pred in o["best_set"]
            correct.append(ok)
            top1.append(1.0 if ok else 0.0)
            # top-2: if we have full scores
            scores = pred_fn(k, scores=True)
            if scores:
                ordered = sorted(scores, key=scores.get)
                top2.append(1.0 if any(L in o["best_set"] for L in ordered[:2]) else 0.0)
                taus.append(kendall_tau(scores, o["times"]))
            t_pred = o["times"].get(pred)
            t_best = o["times"][o["best"]]
            if t_pred and t_best:
                regret.append(t_pred / t_best)
            if "CSR" in o["times"] and t_pred:
                speedup_vs_csr.append(o["times"]["CSR"] / t_pred)
        return {
            "top1": bootstrap_ci(top1),
            "top2": bootstrap_ci(top2) if top2 else None,
            "kendall_tau_mean": float(np.nanmean(taus)) if taus else None,
            "regret_geomean": float(np.exp(np.mean(np.log(regret)))) if regret else None,
            "regret_cdf": [float(np.quantile(regret, q)) for q in (0.5, 0.9, 0.95)]
            if regret
            else None,
            "speedup_vs_csr_geomean": float(np.exp(np.mean(np.log(speedup_vs_csr))))
            if speedup_vs_csr
            else None,
            "correct": correct,
        }

    def at_pred(k, scores=False):
        r = at_by[k]
        if scores:
            return {
                L: float(r.get(f"cost_{L}", 1e100))
                for L in LAYOUTS
                if r.get(f"cost_{L}") not in (None, "", "-1", -1)
                and float(r.get(f"cost_{L}", -1)) >= 0
            }
        return r.get("local_argmin") or r.get("dp_chosen")

    def wg_pred(k, scores=False):
        r = wg_by[k]
        if scores:
            return {L: float(r[f"score_{L}"]) for L in LAYOUTS}
        return r["local_argmin"]

    at_eval = eval_tool(at_pred)
    wg_eval = eval_tool(wg_pred)
    report["autotuner_vs_oracle"] = {k: v for k, v in at_eval.items() if k != "correct"}
    report["wacog_vs_oracle"] = {k: v for k, v in wg_eval.items() if k != "correct"}
    report["mcnemar"] = mcnemar(at_eval["correct"], wg_eval["correct"])

    # Breakdowns
    breakdowns = defaultdict(lambda: {"at": [], "wg": []})
    for k in keys:
        r = at_by[k]
        fam = r.get("family") or "unknown"
        kind = r.get("dominant") or r.get("kind")
        o = oracle[k]
        for label in (f"family:{fam}", f"kind:{kind}"):
            breakdowns[label]["at"].append(
                1.0 if at_pred(k) in o["best_set"] else 0.0
            )
            breakdowns[label]["wg"].append(
                1.0 if wg_pred(k) in o["best_set"] else 0.0
            )
    report["breakdowns"] = {
        k: {
            "at_top1": float(np.mean(v["at"])) if v["at"] else None,
            "wacog_top1": float(np.mean(v["wg"])) if v["wg"] else None,
            "n": len(v["at"]),
        }
        for k, v in breakdowns.items()
    }

    # Track 2
    if art:
        art_ok = []
        for r in art:
            if not r.get("oracle_csr_bcsr"):
                continue
            art_ok.append(1.0 if r["waco_overlap_pred"] == r["oracle_csr_bcsr"] else 0.0)
        report["track2_overlap"] = {
            "n": len(art_ok),
            "top1_csr_bcsr": bootstrap_ci(art_ok) if art_ok else None,
        }

    report["threats_to_validity"] = [
        "WACO-G uses a WACONet-inspired CNN without MinkowskiEngine sparse convolution; results are methodological, not a bit-exact WACO reproduction.",
        "Corpus is synthetic (ER/BA/RMAT) resized for laptop GPU/CPU limits, smaller than WACO's 726 SuiteSparse test matrices.",
        "Track 2 format mapping from SuperSchedule level formats to CSR/BCSR is approximate.",
        "AutoTuner conversion costs in end-to-end DP use the compiler's analytic model; WACO-G DP uses a proxy conversion term.",
        "Hardware is AMD Ryzen 7 4800H; WACO paper used Xeon+icc — models are retrained here.",
    ]

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2))

    # Markdown summary
    lines = [
        "# WACO vs ARS AutoTuner — region-level comparison",
        "",
        f"Regions compared: **{report['n_regions']}**",
        "",
        "## Agreement (AutoTuner local_argmin vs WACO-G)",
        f"- match rate: {report['agreement']['match_rate']:.3f}",
        f"- Cohen's kappa: {report['agreement']['cohen_kappa']:.3f}",
        "",
        "## Accuracy vs measured oracle (kernel-only)",
        f"- AutoTuner top-1: {report['autotuner_vs_oracle']['top1']}",
        f"- WACO-G top-1: {report['wacog_vs_oracle']['top1']}",
        f"- AutoTuner regret geomean: {report['autotuner_vs_oracle']['regret_geomean']}",
        f"- WACO-G regret geomean: {report['wacog_vs_oracle']['regret_geomean']}",
        f"- McNemar: {report['mcnemar']}",
        "",
        "## Threats to validity",
    ]
    lines += [f"- {t}" for t in report["threats_to_validity"]]
    args.md.write_text("\n".join(lines) + "\n")
    print(json.dumps(report, indent=2))
    print(f"wrote {args.out}")
    print(f"wrote {args.md}")


if __name__ == "__main__":
    main()
