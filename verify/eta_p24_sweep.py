#!/usr/bin/env python3
"""P = 1..24 predicted/measured sweep for the DOALL and DOACROSS cost models.

For each thread count P:
  DOALL    : run doall_scaling on g20k (200 rounds, N=20000), min of 3 wall
             times, one GRAPH_PARALLEL_DEBUG run for the calibrated fields.
             pred_serial = c_ns*N
             pred_P      = L_total(P) + c_ns*N/P
  DOACROSS : run doacross_repeat_d1 (200 dispatches, N=8191), min of 3 wall
             times unforced (the model's own decision, expected serial -- the
             chain cannot be beaten) and min of 3 with SGPL_FORCE_DOACROSS_PARALLEL
             (the parallel path measured directly), one debug run for the
             calibrated fields.
             pred_serial = N*(c_dep+c_ind)
             pred_P      = L_total + N*c_dep + N*c_ind/P + N*sync_per_iter
  measured_per_dispatch = t_wall_s / rounds
  eta = predicted / measured   (1.00 = perfect)

Writes eta_p24.csv, eta_p24_table.md, eta_p24.png/.svg.
"""
import csv
import os
import re
import subprocess

REPO = "/workspace/IMTalker/p1/p1GraphEasy-con-AutoTuner"
VERIFY = "/workspace/IMTalker/p1/verify"
OUT = os.path.join(VERIFY, "bin", "p24")
P_LIST = list(range(1, 25))

KNOBS_DOACROSS = {"SGPL_OUTLINER_MIN_EFF": "1", "SGPL_OUTLINER_MIN_TRIP": "2"}


def build(graph, extra=None):
    env = dict(os.environ)
    env.update(KNOBS_DOACROSS if extra else {})
    env["SGPL_GPU_BACKEND"] = "0"
    env["GRAPH_FILE"] = graph
    for f in ("final_program", "program.o", "gpu_runtime.o"):
        p = os.path.join(REPO, f)
        if os.path.exists(p):
            os.unlink(p)
    with open("/tmp/p24_build.log", "w") as log:
        r = subprocess.run(["bash", "03_run.sh"], cwd=REPO, env=env,
                           stdout=log, stderr=subprocess.STDOUT)
    return r.returncode == 0 and os.path.exists(os.path.join(REPO, "final_program"))


def run(env_extra, timeout=900):
    env = dict(os.environ)
    env.update(env_extra)
    p = subprocess.run(["./final_program"], cwd=REPO, env=env,
                       capture_output=True, text=True, timeout=timeout)
    return p


def best_of(n, env_extra):
    best = None
    for _ in range(n):
        p = run(env_extra)
        t = p.stdout.strip().split("\n")[-1] if p.stdout else ""
        # wall time is measured by the caller via time.perf_counter around run()
        if best is None:
            best = t
    return best


def time_run(env_extra):
    import time
    t0 = time.perf_counter()
    p = run(env_extra)
    dt = time.perf_counter() - t0
    return dt, p


FIELDS = ("N", "P", "c_dep_ns", "c_ind_ns", "L_total_ns", "sigma_wait_ns",
          "sigma_post_ns", "sync_term_per_iter", "choose", "c_state", "samples")


def parse_doacross(stderr):
    lines = [l for l in stderr.split("\n") if "cost-doacross" in l]
    if not lines:
        return None
    d = {}
    for k in FIELDS:
        m = re.search(rf"\b{k}=([A-Za-z0-9._-]+)", lines[-1])
        if m:
            d[k] = m.group(1)
    return d


def parse_doall(stderr, P):
    lines = [l for l in stderr.split("\n") if "cost-doall" in l]
    if not lines:
        return None
    d = {}
    for k in ("N", "P", "c_ns", "L_total_ns", "choose", "c_state", "samples"):
        m = re.search(rf"\b{k}=([A-Za-z0-9._-]+)", lines[-1])
        if m:
            d[k] = m.group(1)
    return d


def doall_rows():
    graph = os.path.join(VERIFY, "bin", "doall", "doall_g20k.graph")
    if not build(graph):
        return []
    rows = []
    c_ns_anchor = None
    for P in P_LIST:
        base = {"SGPL_NUM_THREADS": str(P)}
        times = []
        for _ in range(3):
            dt, _ = time_run(base)
            times.append(dt)
        t_best = min(times)
        p = run({**base, "GRAPH_PARALLEL_DEBUG": "1"})
        d = parse_doall(p.stderr, P) or {}
        try:
            c_ns = float(d.get("c_ns", "nan"))
        except ValueError:
            c_ns = float("nan")
        if c_ns == c_ns and c_ns > 0:
            c_ns_anchor = c_ns
        use_c = c_ns if (c_ns == c_ns and c_ns > 0) else (c_ns_anchor or float("nan"))
        try:
            L = float(d.get("L_total_ns", "nan"))
        except ValueError:
            L = float("nan")
        if L != L or L <= 0:
            L = 0.0
        N = float(d.get("N", 20000) or 20000)
        pred_serial_ms = use_c * N / 1e6
        pred_ms = (L + use_c * N / P) / 1e6
        meas_ms = t_best / 200.0 * 1e3
        rows.append({"model": "doall", "P": P, "N": int(N),
                     "c_dep_ns": "", "c_ind_ns": "",
                     "c_ns": f"{use_c:.3f}", "L_total_ns": f"{L:.1f}",
                     "sync_per_iter_ns": "",
                     "pred_serial_ms": f"{pred_serial_ms:.4f}",
                     "pred_P_ms": f"{pred_ms:.4f}",
                     "measured_ms": f"{meas_ms:.4f}",
                     "eta": f"{pred_ms / meas_ms:.3f}",
                     "choose": d.get("choose", "-"),
                     "c_state": d.get("c_state", "-")})
        print(f"doall P={P:2d} pred={pred_ms:8.4f} ms meas={meas_ms:8.4f} ms "
              f"eta={pred_ms / meas_ms:6.3f}", flush=True)
    return rows


def doacross_rows():
    graph = os.path.join(VERIFY, "cases", "parallel", "doacross_repeat_d1.graph")
    if not build(graph, extra=True):
        return []
    rows = []
    for P in P_LIST:
        base = {"SGPL_NUM_THREADS": str(P)}
        # unforced: the model's own decision runs (serial for a chain)
        t_serial = min(time_run(base)[0] for _ in range(3))
        # forced parallel: the parallel path measured directly
        t_par = min(time_run({**base, "SGPL_FORCE_DOACROSS_PARALLEL": "1"})[0]
                    for _ in range(3))
        p = run({**base, "GRAPH_PARALLEL_DEBUG": "1",
                 "SGPL_NO_WARMUP_DECISION_CACHE": "1"})
        d = parse_doacross(p.stderr) or {}
        def fv(k, default=0.0):
            try:
                return float(d.get(k, default))
            except ValueError:
                return default
        N = fv("N", 8191)
        c_dep = fv("c_dep_ns")
        c_ind = fv("c_ind_ns")
        sync = fv("sync_term_per_iter")
        L = fv("L_total_ns")
        pred_serial_ms = N * (c_dep + c_ind) / 1e6
        pred_P_ms = (L + N * c_dep + N * c_ind / P + N * sync) / 1e6
        meas_serial_ms = t_serial / 200.0 * 1e3
        meas_par_ms = t_par / 200.0 * 1e3
        rows.append({"model": "doacross", "P": P, "N": int(N),
                     "c_dep_ns": f"{c_dep:.3f}", "c_ind_ns": f"{c_ind:.3f}",
                     "c_ns": "", "L_total_ns": f"{L:.1f}",
                     "sync_per_iter_ns": f"{sync:.2f}",
                     "pred_serial_ms": f"{pred_serial_ms:.4f}",
                     "pred_P_ms": f"{pred_P_ms:.4f}",
                     "measured_ms": f"{meas_par_ms:.4f}",
                     "measured_serial_ms": f"{meas_serial_ms:.4f}",
                     "eta": f"{pred_P_ms / meas_par_ms:.3f}",
                     "eta_serial": f"{pred_serial_ms / meas_serial_ms:.3f}",
                     "choose": d.get("choose", "-"),
                     "c_state": d.get("c_state", "-")})
        print(f"doacross P={P:2d} pred={pred_P_ms:8.4f} ms meas(forced)={meas_par_ms:8.4f} "
              f"meas(serial)={meas_serial_ms:8.4f} eta={pred_P_ms / meas_par_ms:6.3f} "
              f"eta_serial={pred_serial_ms / meas_serial_ms:6.3f}", flush=True)
    return rows


def main():
    os.makedirs(OUT, exist_ok=True)
    rows = doall_rows() + doacross_rows()
    if not rows:
        print("no rows produced")
        return 1
    keys = ["model", "P", "N", "c_ns", "c_dep_ns", "c_ind_ns", "L_total_ns",
            "sync_per_iter_ns", "pred_serial_ms", "pred_P_ms", "measured_ms",
            "measured_serial_ms", "eta", "eta_serial", "choose", "c_state"]
    csv_path = os.path.join(OUT, "eta_p24.csv")
    with open(csv_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=keys, extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)

    md = ["# Predicted vs measured, P = 1..24 (pod CPU)",
          "",
          "eta = predicted / measured per dispatch (1.00x = perfect). "
          "DOALL: pred_P = L(P) + c_ns*N/P. "
          "DOACROSS: pred_P = L + N*c_dep + N*c_ind/P + N*sync_per_iter.", "",
          "| model | P | pred (ms) | measured (ms) | eta | note |",
          "|---|---|---|---|---|---|"]
    for r in rows:
        note = ""
        if r["model"] == "doacross":
            note = f"forced parallel; unforced measured {r.get('measured_serial_ms', '-')} ms (eta_serial {r.get('eta_serial', '-')})"
        md.append(f"| {r['model']} | {r['P']} | {r['pred_P_ms']} | {r['measured_ms']} | {r['eta']} | {note} |")
    with open(os.path.join(OUT, "eta_p24_table.md"), "w") as f:
        f.write("\n".join(md) + "\n")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, axes = plt.subplots(1, 2, figsize=(13.5, 5.4))
        for ax, model, title in ((axes[0], "doall", "DOALL (doall_scaling, N=20000)"),
                                 (axes[1], "doacross", "DOACROSS (repeat_d1, N=8191, forced parallel)")):
            rs = [r for r in rows if r["model"] == model]
            if not rs:
                continue
            Ps = [int(r["P"]) for r in rs]
            pred = [float(r["pred_P_ms"]) for r in rs]
            meas = [float(r["measured_ms"]) for r in rs]
            eta = [float(r["eta"]) for r in rs]
            ax.plot(Ps, pred, "o-", label="predicted")
            ax.plot(Ps, meas, "s-", label="measured")
            ax.set_xlabel("threads P")
            ax.set_ylabel("ms per dispatch")
            ax.set_title(title)
            ax.grid(True, alpha=0.3)
            ax2 = ax.twinx()
            ax2.plot(Ps, eta, "k--", lw=1.2, label="eta (right)")
            ax2.axhline(1.0, color="gray", ls=":", lw=1)
            ax2.set_ylabel("eta = predicted / measured")
            h1, l1 = ax.get_legend_handles_labels()
            h2, l2 = ax2.get_legend_handles_labels()
            ax.legend(h1 + h2, l1 + l2, fontsize=8, loc="best")
        fig.suptitle("Cost-model prediction vs measurement, P = 1..24 "
                     "(runpod CPU, best of 3)")
        fig.tight_layout()
        fig.savefig(os.path.join(OUT, "eta_p24.png"), dpi=140)
        fig.savefig(os.path.join(OUT, "eta_p24.svg"))
        print("figure written")
    except Exception as e:
        import traceback
        traceback.print_exc()
    print("wrote", csv_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
