#!/usr/bin/env python3
"""Update CSR+BCSR Insert predicted columns of the BA/ER CSVs with the
current (phantom-realloc-free) hybrid model; recompute Insert verdicts."""
import csv
import shutil
import sys

sys.path.insert(0, ".")
import cost_model as cm

cm.set_cache_model("hybrid")
LAY = ["CSR", "PCSR", "BCSR", "SET"]


def ffloat(s):
    s = str(s).strip()
    if not s or s.lower() == "none":
        return None
    return float(s)


def verdict(preds, mm, mi):
    valid = [(l, preds[l], mm[l]) for l in LAY if mm.get(l) is not None]
    if not valid:
        return "NODATA", "-", "-"
    bp = min(valid, key=lambda x: x[1])[0]
    bm = min(valid, key=lambda x: x[2])[0]
    ps = sorted(p for _, p, _ in valid)
    pt = (ps[1] - ps[0]) <= 0.05 * max(ps[0], 1.0)
    ms = sorted((m_, l_) for l_, _, m_ in valid)
    a = mi.get(ms[0][1], 0) or 0
    b = mi.get(ms[1][1], 0) or 0
    mt = abs(ms[0][0] - ms[1][0]) < max(a, b)
    if pt and mt:
        return "TIE", bp, bm
    if bp == bm:
        return "MATCH", bp, bm
    if mt:
        return "NOISE", bp, bm
    return "MISMATCH", bp, bm


def main():
    for csv_name in ("barabasi_albert_runs10.csv", "erdos_renyi_runs10.csv"):
        shutil.copy2(csv_name, csv_name + ".prephantomfix.bak")
        rows = list(csv.DictReader(open(csv_name)))
        for r in rows:
            if r["operation"] == "Insert" and r["layout"] in ("CSR", "BCSR"):
                cm._CUR_GRAPH = r["graph"]
                per = cm.predicted_ns(
                    r["layout"], "Insert",
                    int(float(r["n_vertices"])), int(float(r["m_undirected"])))
                r["predicted_ns"] = f"{50.0 * per:.1f}"
        for (g, op) in {(r["graph"], r["operation"]) for r in rows}:
            if op != "Insert":
                continue
            mm = {l: ffloat(next((x["measured_kernel_ns"] for x in rows
                                  if x["graph"] == g and x["operation"] == op
                                  and x["layout"] == l), None)) for l in LAY}
            mi = {l: ffloat(next((x["measured_iqr_ns"] for x in rows
                                  if x["graph"] == g and x["operation"] == op
                                  and x["layout"] == l), 0)) for l in LAY}
            pr = {l: float(next(x["predicted_ns"] for x in rows
                                if x["graph"] == g and x["operation"] == op
                                and x["layout"] == l)) for l in LAY}
            v, bp, bm = verdict(pr, mm, mi)
            for x in rows:
                if x["graph"] == g and x["operation"] == op:
                    x["predicted_best"] = bp
                    x["measured_best"] = bm
                    x["verdict"] = v if x["layout"] == "CSR" else ""
        with open(csv_name, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=rows[0].keys())
            w.writeheader()
            w.writerows(rows)
        from collections import Counter
        print("updated", csv_name,
              Counter(r["verdict"] for r in rows if r["verdict"]))


if __name__ == "__main__":
    main()