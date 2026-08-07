import csv
from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.colors import hex2color


LAYOUT_ORDER = ["CSR", "PCSR", "BCSR", "SET"]


def read_csv(path):
    graphs = {}
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            g = row["graph"]
            op = row.get("operation") or row.get("op")
            lay = row["layout"]
            pred_s = row["predicted_ns"]
            pred = float(pred_s) if pred_s else None
            meas_s = row.get("measured_ns")
            if meas_s is None:
                meas_s = row.get("measured_kernel_ns")
            meas = int(meas_s) if meas_s else None

            key = (g, op)
            if key not in graphs:
                graphs[key] = {}
            graphs[key][lay] = (pred, meas)
    return graphs


def ranking_text(pairs):
    """
    Given list of (layout, value), sort by value ascending,
    return formatted string like '1.CSR 2.PCSR 3.BCSR 4.SET'
    Layouts with None (timed-out) are shown as 'x.LAY(T/O)'.
    """
    valid = [(l, v) for l, v in pairs if v is not None]
    timed_out = [(l, v) for l, v in pairs if v is None]
    valid_sorted = sorted(valid, key=lambda x: x[1])
    labelled = []
    for i, (lay, val) in enumerate(valid_sorted):
        labelled.append(f"{i+1}.{lay}")
    for lay, _ in timed_out:
        labelled.append(f"x.{lay}(T/O)")
    return "  ".join(labelled)


def ranking_order(pairs):
    """Return tuple of layout names sorted by value ascending.
    Skip None values (timed-out/crashed layouts)."""
    valid = [(l, v) for l, v in pairs if v is not None]
    return tuple(p[0] for p in sorted(valid, key=lambda x: x[1] if x[1] is not None else float('inf')))


def make_table(rows, title, output_path):
    headers = [
        "Graph / Op",
        "n",
        "m",
        "Predicted ranking", "Measured ranking", "Verdict",
    ]

    row_data = []
    row_bg = []

    for (g, op), nv, md, pred_pairs, meas_pairs in rows:
        pred_rank = ranking_text(pred_pairs)
        meas_rank = ranking_text(meas_pairs)

        # Only compare rankings for layouts with valid measured data
        valid_meas = {l for l, v in meas_pairs if v is not None}
        pred_filtered = [(l, v) for l, v in pred_pairs if l in valid_meas]
        meas_filtered = [(l, v) for l, v in meas_pairs if l in valid_meas]

        pred_order = ranking_order(pred_filtered)
        meas_order = ranking_order(meas_filtered)

        has_any_meas = bool(valid_meas)
        has_all_meas = len(valid_meas) == len(LAYOUT_ORDER)
        # Tie-tolerance: when the top-2 predicted layouts are within 5%, the
        # prediction considers them tied and a different measured ordering is
        # not a real mismatch — report PRED_TIE instead.  (A full MEAS_TIE
        # needs IQRs, which ranking_table.py doesn't have; bench_folder.py
        # emits those.)
        PRED_TIE_EPS = 0.05
        pred_vals = sorted(v for _, v in pred_filtered if v is not None)
        pred_tie = (len(pred_vals) >= 2 and
                    (pred_vals[1] - pred_vals[0]) <= PRED_TIE_EPS * max(pred_vals[0], 1.0))
        if not has_any_meas:
            match_text = "N/A"
        elif pred_order == meas_order:
            match_text = "MATCH"
        elif pred_tie:
            match_text = "PRED_TIE"
        else:
            match_text = "MISMATCH"

        row_data.append([f"{g} {op}", nv, md, pred_rank, meas_rank, match_text])

        if match_text == "MATCH":
            bg = ["w"] * 5 + ["#c8f7c5"]
        elif match_text == "MISMATCH":
            bg = ["w"] * 5 + ["#f7c5c5"]
        elif match_text == "PRED_TIE":
            bg = ["w"] * 5 + ["#f7e8c5"]
        else:
            bg = ["w"] * 5 + ["#e0e0e0"]
        row_bg.append(bg)

    nrows = len(row_data) + 1
    fig, ax = plt.subplots(figsize=(max(14, nrows * 0.9), 0.5 + 0.35 * nrows))
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
    tbl.set_fontsize(max(5.5, min(9, 140 / len(headers))))
    tbl.scale(1, 1.5)

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


def process_csv(csv_path, title):
    print(f"Processing {csv_path.name} ...")
    data = read_csv(csv_path)

    # Gather rows with n, m
    rows = []
    for (g, op), layouts in sorted(data.items()):
        # Extract n, m from first layout's graph name
        # We'll read them from the raw CSV re-read
        pass

    # Re-read to get n_vertices, m_directed
    nv_map = {}
    md_map = {}
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            key = (row["graph"], row.get("operation") or row["op"])
            if key not in nv_map:
                n_col = row.get("n") or row.get("n_vertices")
                m_col = row.get("m") or row.get("m_directed") or row.get("m_undirected")
                nv_map[key] = (int(n_col), int(m_col))

    for (g, op), layouts in sorted(data.items()):
        nv, md = nv_map[(g, op)]

        pred_pairs = []
        meas_pairs = []
        for lay in LAYOUT_ORDER:
            if lay in layouts:
                pred, meas = layouts[lay]
                pred_pairs.append((lay, pred))
                meas_pairs.append((lay, meas))

        rows.append(((g, op), nv, md, pred_pairs, meas_pairs))

    out_path = csv_path.parent / csv_path.name.replace(".csv", "_ranking.png")
    make_table(rows, title, out_path)


def main():
    import sys
    csv_dir = Path(__file__).resolve().parent

    if len(sys.argv) > 1:
        # Process a single CSV passed as CLI argument
        csv_path = Path(sys.argv[1]).resolve()
        if not csv_path.exists():
            print(f"Error: {csv_path} not found", file=sys.stderr)
            sys.exit(1)
        title = sys.argv[2] if len(sys.argv) > 2 else csv_path.stem
        process_csv(csv_path, title)
    else:
        datasets = [
            ("erdos_renyi.csv", "Erdos-Renyi Synthetic Graphs"),
            ("barabasi_albert.csv", "Barabasi-Albert Synthetic Graphs"),
            ("real_graphs.csv", "Real-World Graphs"),
        ]

        for csv_name, title in datasets:
            csv_path = csv_dir / csv_name
            if not csv_path.exists():
                print(f"  SKIP (not found): {csv_path}")
                continue
            process_csv(csv_path, title)


if __name__ == "__main__":
    main()
