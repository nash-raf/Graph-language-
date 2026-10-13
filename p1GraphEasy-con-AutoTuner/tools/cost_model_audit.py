#!/usr/bin/env python3
"""Build and record local, warm, loop-only probes; run under Linux/WSL.

Outputs raw CSV-shaped records and environment metadata. Predictions are
obtained from the actual C helpers. Existing result artifacts are not changed.
"""
import argparse
import os
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]


def build(out):
    for source, compiler, name in (
        (ROOT / "tools/cost_model_probe.c", "gcc", "probe"),
        (ROOT / "roaring_bitmap.cpp", "g++", "bitmap"),
    ):
        subprocess.run([compiler, "-O3", "-fopenmp", "-mavx2", "-pthread",
                        "-c", str(source), "-o", str(out / (name + ".o"))], check=True)
    binary = out / "probe"
    subprocess.run(["g++", "-fopenmp", "-pthread", str(out / "probe.o"),
                    str(out / "bitmap.o"), "-lnlopt", "-lm", "-o", str(binary)], check=True)
    return binary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=pathlib.Path, required=True)
    parser.add_argument("--quick", action="store_true")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    binary = build(args.out)
    metadata = subprocess.run(["lscpu"], capture_output=True, text=True, check=True).stdout
    (args.out / "environment.txt").write_text(metadata)
    cases = [(20000, 12, 0, 1), (200000, 24, 0, 1), (200000, 0, 1, 4)]
    if not args.quick:
        cases += [(8192, 0, 1, 1), (65536, 4, 0, 4), (1000000, 12, 0, 4)]
    with (args.out / "raw.csv").open("w") as output:
        output.write("# all: P,N,chain,streaming,serial_ns,predicted_ns,parallel_ns,launch_pred_ns,launch_measured_ns\n")
        output.write("# across: P,N,distance,dep_interval_ns,serial_profile_wall_ns,parallel_ns,launch_pred_ns,launch_measured_ns,outside_clock_ns_per_iter,decision\n")
        output.write("# across-proposal: P,N,distance,proposal_ns,parallel_ns,calibrated_handoff_ns,useful_c_ns\n")
        output.write("# pair: P,N,chain,streaming,ratio,w1,w2,alone1_ns,alone2_ns,sum_ns,resource_ns,paired_ns\n")
        for p in (2, 4, 8):
            for case in cases:
                env = dict(os.environ, SGPL_NUM_THREADS=str(p), OMP_NUM_THREADS=str(p))
                for key in ("GRAPH_PARALLEL_DEBUG", "SGPL_TDG_DEBUG", "SGPL_BUDGET_DEBUG",
                            "SGPL_FORCE_WIDTHS", "SGPL_FORCE_DOALL_PARALLEL", "SGPL_FORCE_DOACROSS_PARALLEL"):
                    env.pop(key, None)
                result = subprocess.run([str(binary), *map(str, case)], env=env,
                                        capture_output=True, text=True, check=True, timeout=90)
                output.write(result.stdout)
                output.flush()
                print(f"P={p} case={case}: " + result.stdout.splitlines()[0], flush=True)
                if result.stderr:
                    with (args.out / "stderr.log").open("a") as errors:
                        errors.write(result.stderr)


if __name__ == "__main__":
    main()
