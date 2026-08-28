#!/usr/bin/env python3
"""Ranking tables with density + measured near-tie gap columns."""
import csv
import re
from pathlib import Path
import matplotlib.pyplot as plt

LAYOUT_ORDER = ["CSR", "PCSR", "BCSR", "SET"]
SCRIPT_DIR = Path(__file__).resolve().parent


def read_csv(path):
    graphs = {}
    meta = {}
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            g = row["graph"]
            op = row.get("operation") or row.get("op")
            lay = row["layout"]
            pred_s = row.get("predicted_ns") or ""
            pred = float(pred_s) if pred_s.strip() else None
            meas_s = row.get("measured_ns")
            if meas_s is None:
                meas_s = row.get("measured_kernel_ns")
            meas = int(meas_s) if meas_s and str(meas_s).strip() else None

            key = (g, op)
            if key not in graphs:
                graphs[key] = {}
                n_col = row.get("n") or row.get("n_vertices")
                m_col = (row.get("m") or row.get("m_undirected")
                         or row.get("m_directed"))
                meta[key] = (int(n_col), int(m_col),
                             "m_directed" in row and "m_undirected" not in row)
            graphs[key][lay] = (pred, meas)
    return graphs, meta


def density(n, m, m_is_directed):
    undirected = m // 2 if m_is_directed else m
    if n <= 1:
        return 0.0
    return 2.0 * undirected / (n * (n - 1.0))


def short_graph_name(g):
    """Compact graph label. Prefer BA m-param or ER density tag over full synth_n* prefix."""
    # BA: synth_n5000_m128_d0.049899 → m128
    m = re.search(r"_m(\d+)_d", g)
    if m:
        return f"m{m.group(1)}"
    # ER: synth_n5000_d0.05 → d0.05
    d = re.search(r"_d([0-9.]+)$", g)
    if d:
        return f"d{d.group(1)}"
    return g


def ranking_text(pairs):
    valid = [(l, v) for l, v in pairs if v is not None]
    timed_out = [(l, v) for l, v in pairs if v is None]
    valid_sorted = sorted(valid, key=lambda x: x[1])
    labelled = [f"{i+1}.{lay}" for i, (lay, _) in enumerate(valid_sorted)]
    for lay, _ in timed_out:
        labelled.append(f"x.{lay}(T/O)")
    return "  ".join(labelled)


def ranking_order(pairs):
    valid = [(l, v) for l, v in pairs if v is not None]
    return tuple(p[0] for p in sorted(valid, key=lambda x: x[1]))


def near_tie_text(meas_pairs):
    """% gap between the two fastest measured layouts (near-tie magnitude)."""
    valid = sorted([(l, v) for l, v in meas_pairs if v is not None],
                   key=lambda x: x[1])
    if len(valid) < 2:
        return "—"
    (l1, t1), (l2, t2) = valid[0], valid[1]
    pct = 100.0 * (t2 - t1) / max(t1, 1)
    return f"{pct:.1f}% ({l1}/{l2})"


def mismatch_gap_text(pred_order, meas_order, meas_map):
    """Measured % time gap between every layout pair whose relative order is
    inverted in the prediction vs the measurement (the pairs that cause the
    MISMATCH).  gap = 100 * (slower - faster) / faster."""
    meas_map = {l: v for l, v in meas_map.items() if v is not None}
    pos_p = {l: i for i, l in enumerate(pred_order)}
    pos_m = {l: i for i, l in enumerate(meas_order)}
    parts = []
    for i, u in enumerate(pred_order):
        if u not in pos_m:
            continue
        for v in pred_order[i + 1:]:
            if v not in pos_m:
                continue
            if pos_m[u] > pos_m[v]:
                t1, t2 = meas_map[u], meas_map[v]
                fast, slow = min(t1, t2), max(t1, t2)
                pct = 100.0 * (slow - fast) / max(fast, 1)
                parts.append(f"{v}/{u} {pct:.1f}%")
    return " | ".join(parts) if parts else near_tie_text(
        [(l, meas_map[l]) for l in pred_order])


def make_table(rows, title, output_path):
    headers = [
        "Graph / Op", "n", "m", "density",
        "Predicted ranking", "Measured ranking",
        "Meas. near-tie Δ%", "Verdict",
    ]
    row_data = []
    row_bg = []

    for (g, op), nv, md, dens, pred_pairs, meas_pairs in rows:
        pred_rank = ranking_text(pred_pairs)
        meas_rank = ranking_text(meas_pairs)

        valid_meas = {l for l, v in meas_pairs if v is not None}
        pred_filtered = [(l, v) for l, v in pred_pairs if l in valid_meas]
        meas_filtered = [(l, v) for l, v in meas_pairs if l in valid_meas]
        pred_order = ranking_order(pred_filtered)
        meas_order = ranking_order(meas_filtered)

        PRED_TIE_EPS = 0.05
        pred_vals = sorted(v for _, v in pred_filtered if v is not None)
        pred_tie = (len(pred_vals) >= 2 and
                    (pred_vals[1] - pred_vals[0]) <= PRED_TIE_EPS * max(pred_vals[0], 1.0))

        if not valid_meas:
            match_text = "N/A"
        elif pred_order == meas_order:
            match_text = "MATCH"
        elif pred_tie:
            match_text = "PRED_TIE"
        else:
            match_text = "MISMATCH"

        if match_text == "MISMATCH":
            meas_map = dict(meas_pairs)
            tie_txt = mismatch_gap_text(pred_order, meas_order, meas_map)
        else:
            tie_txt = near_tie_text(meas_pairs)

        dens_s = f"{dens:.4f}" if dens >= 0.01 else f"{dens:.6f}"
        row_data.append([
            f"{short_graph_name(g)} {op}",
            f"{nv:,}", f"{md:,}", dens_s,
            pred_rank, meas_rank, tie_txt, match_text,
        ])

        ncols = len(headers)
        if match_text == "MATCH":
            bg = ["w"] * (ncols - 1) + ["#c8f7c5"]
        elif match_text == "MISMATCH":
            bg = ["w"] * (ncols - 1) + ["#f7c5c5"]
        elif match_text == "PRED_TIE":
            bg = ["w"] * (ncols - 1) + ["#f7e8c5"]
        else:
            bg = ["w"] * (ncols - 1) + ["#e0e0e0"]
        row_bg.append(bg)

    nrows = len(row_data) + 1
    fig, ax = plt.subplots(figsize=(18, 0.55 + 0.38 * nrows))
    ax.axis("off")

    table_data = [headers] + row_data
    cell_colours = [["#404040"] * len(headers)] + row_bg

    tbl = ax.table(
        cellText=table_data,
        cellColours=cell_colours,
        loc="center",
        cellLoc="center",
    )
    tbl.auto_set_font_size(False)
    tbl.set_fontsize(7.5)
    tbl.scale(1, 1.55)

    # Column width hints (relative)
    col_widths = [0.16, 0.07, 0.09, 0.08, 0.22, 0.22, 0.12, 0.08]
    for j, w in enumerate(col_widths):
        for i in range(len(table_data)):
            tbl[i, j].set_width(w)

    for j in range(len(headers)):
        cell = tbl[0, j]
        cell.set_text_props(color="white", fontweight="bold")
        cell.set_facecolor("#404040")

    for i in range(1, len(table_data)):
        for j in range(len(headers)):
            cell = tbl[i, j]
            cell.set_edgecolor("#d0d0d0")
            cell.set_linewidth(0.5)

    ax.set_title(title, fontsize=14, fontweight="bold", pad=20)
    fig.tight_layout()
    fig.savefig(output_path, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"  Wrote {output_path}")


def process_csv(csv_path, title, out_name=None):
    print(f"Processing {csv_path.name} ...")
    data, meta = read_csv(csv_path)
    rows = []
    for (g, op), layouts in sorted(data.items()):
        nv, md, m_dir = meta[(g, op)]
        dens = density(nv, md, m_dir)
        pred_pairs = []
        meas_pairs = []
        for lay in LAYOUT_ORDER:
            if lay in layouts:
                pred, meas = layouts[lay]
                pred_pairs.append((lay, pred))
                meas_pairs.append((lay, meas))
        rows.append(((g, op), nv, md, dens, pred_pairs, meas_pairs))

    out = csv_path.parent / (out_name or f"{csv_path.stem}_density_ranking.png")
    make_table(rows, title, out)


def main():
    import sys
    if len(sys.argv) > 1:
        targets = [(Path(a), " ".join(Path(a).stem.split("_")),
                    None) for a in sys.argv[1:]]
    else:
        targets = [
        (SCRIPT_DIR / "erdos_renyi_runs10.csv", "Erdos-Renyi",
         "erdos_renyi_density_ranking.png"),
        (SCRIPT_DIR / "barabasi_albert_runs10.csv", "Barabasi-Albert",
         "barabasi_albert_density_ranking.png"),
        (SCRIPT_DIR / "meh_validate_updated.csv", "Real World Graphs",
         "real_world_graphs_density_ranking.png"),
    ]
    for path, title, out_name in targets:
        if not path.exists():
            print(f"  SKIP (not found): {path}")
            continue
        process_csv(path, title, out_name)


if __name__ == "__main__":
    main()
