"""validate_analytic.py — intermediate validation (correction 2).

Per class per graph: N_exact, N_analytic, h2_exact, h2_analytic,
h3_exact, h3_analytic (exact = replay oracle via rd_hist/access_class_cost),
then T_analytic vs T_replay vs measured.  Same o conventions on both sides.
"""
import json
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import access_class_cost as acc
import analytic_rd as ard
import rd_hist as rh

TRACES = "/tmp/opencode/traces.json"
MEASURED = "real_world_runs10_merged.csv"

DATASET = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "..", "..", "dataset")

LABEL_TO_FILE = {
    "bio-grid-yeast": ["real graphs", "bio-grid-yeast.txt"],
    "copresence-InVS15": ["real graphs", "copresence-InVS15.txt"],
    "fb-pages-company": ["real graphs", "fb-pages-company.txt"],
    "ia-dbpedia": ["real graphs", "ia-dbpedia.txt"],
    "inf-roadNet-PA": ["real graphs", "inf-roadNet-PA.txt"],
    "road-luxembourg-osm": ["new real", "road-luxembourg-osm.txt"],
    "road-roadNet-PA": ["new real", "road-roadNet-PA.txt"],
    "road-usroads-48": ["new real", "road-usroads-48.txt"],
    "soc-LiveMocha": ["new real", "soc-LiveMocha.txt"],
}


def edge_file(label):
    d, f = LABEL_TO_FILE[label]
    return os.path.join(DATASET, d, f)


def main():
    traces = json.load(open(TRACES))
    rates = ard.class_rates()
    meas = {}
    with open(MEASURED) as f:
        for r in __import__("csv").DictReader(f):
            if r.get("operation") == "Insert" and r.get("layout") == "BCSR":
                meas[r["graph"]] = float(r["measured_kernel_ns"])

    print(f"{'graph':20s} {'cls':>6s} {'N_ex':>8s} {'N_an':>8s} "
          f"{'h2_ex':>6s} {'h2_an':>6s} {'h3_ex':>6s} {'h3_an':>6s}")
    rows = []
    for g, tr in sorted(traces.items()):
        o = tr["bcol_base_mod64"][0] // 16
        o_br = tr["brow_base_mod64"][0] // 16
        n_ops = tr.get("n_ops", 100) or 100
        exact = acc.per_graph_cost(tr, o, o_br)
        st = ard.block_structure_from_edges(edge_file(g), o=o, o_br=o_br)
        # structural check: brow_line must match the trace's op starts
        a, b = rh.per_op_bcol_range(tr["ops"], o)
        starts = np.array([(op["start"] + o) // 16 for op in tr["ops"]])
        blks = np.array([op["blk"] for op in tr["ops"]])
        err = 0
        for blk, s in zip(blks, starts):
            if st["brow_line"][blk] != s:
                err += 1
        an = ard.AnalyticBCSR(st, n_ops=n_ops).expected_tiers()
        for p in ("scan", "move", "brow", "struct"):
            c_ex = exact[p]
            c_an = an[p]
            n_ex = sum(c_ex); n_an = sum(c_an)
            h2e = c_ex[0] / n_ex if n_ex else 0.0
            h3e = (c_ex[0] + c_ex[1]) / n_ex if n_ex else 0.0
            h2a = c_an[0] / n_an if n_an else 0.0
            h3a = (c_an[0] + c_an[1]) / n_an if n_an else 0.0
            sd = ard.AnalyticBCSR(st, n_ops=n_ops).bootstrap_h2_sd(p)
            print(f"{g:20s} {p:>6s} {n_ex:8.0f} {n_an:8.0f} "
                  f"{h2e:6.3f} {h2a:6.3f}+-{sd:.3f} {h3e:6.3f} {h3a:6.3f}")
        if err:
            print(f"{g:20s} !! brow_line mismatch on {err} ops")
        t_replay, _ = acc.cost_of(exact, rates)
        t_an, _ = ard.AnalyticBCSR(st, n_ops=n_ops).cost(rates)
        m_ns = meas.get(g)
        rows.append({
            "graph": g, "n_ops": n_ops,
            "T_replay_ns": t_replay, "T_analytic_ns": t_an,
            "measured_ns": m_ns,
            "ratio_replay": t_replay / m_ns if m_ns else None,
            "ratio_analytic": t_an / m_ns if m_ns else None,
        })
        print(f"{g:20s} T replay {t_replay:12.0f}  analytic {t_an:12.0f}"
              + (f"  meas {m_ns:12.0f}  r_rep {t_replay/m_ns:5.2f}"
                 f"  r_an {t_an/m_ns:5.2f}" if m_ns else ""))

    rp = [r["ratio_replay"] for r in rows if r["ratio_replay"]]
    ra = [r["ratio_analytic"] for r in rows if r["ratio_analytic"]]
    pred_pop = [r["T_analytic_ns"] / r["n_ops"] for r in rows]
    meas_pop = [r["measured_ns"] / r["n_ops"] for r in rows]
    rho = np.corrcoef(np.argsort(np.argsort(pred_pop)).astype(float),
                      np.argsort(np.argsort(meas_pop)).astype(float))[0, 1]
    print(f"\nmedian |ratio-1| replay={np.median(np.abs(np.array(rp)-1)):.3f} "
          f"analytic={np.median(np.abs(np.array(ra)-1)):.3f}  "
          f"Spearman(analytic) rho={rho:.3f}")
    json.dump(rows, open("/tmp/opencode/analytic_validation.json", "w"),
              indent=1)


if __name__ == "__main__":
    main()
