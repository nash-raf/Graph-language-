import os
import glob

src_dir = "synth_graphs_weighted"
dst_dir = "synth_graphs_unweighted"

os.makedirs(dst_dir, exist_ok=True)

for fpath in sorted(glob.glob(os.path.join(src_dir, "*.txt"))):
    fname = os.path.basename(fpath).replace("_w.txt", ".txt")
    out_path = os.path.join(dst_dir, fname)

    with open(fpath) as fin, open(out_path, "w") as fout:
        for line in fin:
            parts = line.split()
            if len(parts) >= 2:
                fout.write(f"{parts[0]} {parts[1]}\n")

    print(f"{fpath} -> {out_path}")

print("Done.")
