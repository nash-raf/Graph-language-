import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def read_csv(path):
    graphs = {}
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            g = row["graph"]
            nv = int(row["n_vertices"])
            md = int(row["m_directed"])
            op = row["operation"]
            layout = row["layout"]
            pred = float(row["predicted_ns"])
            meas = row["measured_kernel_ns"]
            pbest = row["predicted_best"]
            mbest = row["measured_best"]
            verdict = row["verdict"] if row["verdict"] else ""

            if g not in graphs:
                graphs[g] = {"n_vertices": nv, "m_directed": md}
            if op not in graphs[g]:
                graphs[g][op] = {}
            graphs[g][op][layout] = {
                "predicted_ns": pred,
                "measured_kernel_ns": int(meas) if meas else None,
                "predicted_best": pbest,
                "measured_best": mbest,
                "verdict": verdict,
            }
    return graphs


def make_table(csv_path, title, output_path):
    graphs = read_csv(csv_path)
    names = sorted(graphs.keys())

    headers = [
        "Graph",
        "n",
        "m",
        "T pred best",
        "T meas best",
        "Verdict",
        "I pred best",
        "I meas best",
        "Verdict",
    ]
    rows = []
    row_colors = []
    for g in names:
        info = graphs[g]
        nv = info["n_vertices"]
        md = info["m_directed"]

        trav = info.get("Traverse", {})
        ins = info.get("Insert", {})

        t_pred = ""
        t_meas = ""
        t_verd = ""
        for l in trav:
            t_pred = trav[l]["predicted_best"]
            t_meas = trav[l]["measured_best"]
            t_verd = trav[l]["verdict"]
            break

        i_pred = ""
        i_meas = ""
        i_verd = ""
        for l in ins:
            i_pred = ins[l]["predicted_best"]
            i_meas = ins[l]["measured_best"]
            i_verd = ins[l]["verdict"]
            break

        rows.append([g, nv, md, t_pred, t_meas, t_verd, i_pred, i_meas, i_verd])

        color = []
        if t_verd == "MATCH":
            color.extend(["w", "w", "w", "w", "w", "#c8f7c5"])
        elif t_verd == "MISMATCH":
            color.extend(["w", "w", "w", "w", "w", "#f7c5c5"])
        elif t_verd in ("TIE", "PRED_TIE", "NOISE"):
            color.extend(["w", "w", "w", "w", "w", "#f7e8c5"])
        else:
            color.extend(["w", "w", "w", "w", "w", "w"])
        if i_verd == "MATCH":
            color.extend(["w", "w", "#c8f7c5"])
        elif i_verd == "MISMATCH":
            color.extend(["w", "w", "#f7c5c5"])
        elif i_verd in ("TIE", "PRED_TIE", "NOISE"):
            color.extend(["w", "w", "#f7e8c5"])
        else:
            color.extend(["w", "w", "w"])
        row_colors.append(color)

    fig, ax = plt.subplots(figsize=(max(12, len(names) * 1.2), 0.5 + 0.4 * (len(rows) + 1)))
    ax.axis("off")

    table_data = [headers] + rows
    cell_colours = [["#404040"] * len(headers)] + row_colors

    tbl = ax.table(
        cellText=table_data,
        cellColours=cell_colours,
        loc="center",
        cellLoc="center",
    )
    tbl.auto_set_font_size(False)
    tbl.set_fontsize(max(6, min(10, 140 / len(headers))))
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


def main():
    csv_dir = Path(__file__).resolve().parent
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
        out_path = csv_dir / csv_name.replace(".csv", "_table.png")
        print(f"Processing {csv_name} ...")
        make_table(csv_path, title, out_path)


if __name__ == "__main__":
    main()
