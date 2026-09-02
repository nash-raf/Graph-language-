#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
GENERATOR = ROOT / "generate_ablation_workloads.py"
DEFAULT_OUT_DIR = ROOT / "ablation_switch_runs"
TIME_RE = re.compile(r"TIME_SEC=([0-9.]+)\s+RSS_KB=([0-9]+)")
SWITCH_RE = re.compile(r"Injected\s+([0-9]+)\s+layout conversions total")


def run(cmd: list[str], *, cwd: Path, env: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, cwd=cwd, env=env, text=True, capture_output=True, check=True)


def ensure_workloads(out_dir: Path, seeds: str, profile: str) -> list[Path]:
    run(
        [
            sys.executable,
            str(GENERATOR),
            "--out-dir",
            str(out_dir),
            "--seeds",
            seeds,
            "--profile",
            profile,
        ],
        cwd=ROOT,
    )
    return sorted(out_dir.glob("ablation_*.graph"))


def compile_graph(graph_path: Path) -> tuple[int, str]:
    cp = run(["./GraphProgram", str(graph_path)], cwd=ROOT)
    text = (cp.stdout or "") + "\n" + (cp.stderr or "")
    match = SWITCH_RE.search(text)
    return (int(match.group(1)) if match else 0, text)


def time_run(graph_path: Path, threads: int | None) -> tuple[float, int, str]:
    env = os.environ.copy()
    env["GRAPH_FILE"] = os.path.relpath(graph_path, ROOT)
    if threads is not None:
        env["OMP_NUM_THREADS"] = str(threads)

    rp = run(
        ["/usr/bin/time", "-f", "TIME_SEC=%e RSS_KB=%M", "./03_run.sh"],
        cwd=ROOT,
        env=env,
    )
    time_text = (rp.stdout or "") + "\n" + (rp.stderr or "")
    match = TIME_RE.search(time_text)
    if not match:
        raise RuntimeError(f"Could not parse time output for {graph_path.name}")
    return (round(float(match.group(1)) * 1000.0, 3), int(match.group(2)), time_text)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate ablation workloads, find switch-positive autotuner cases, and log time/RSS."
    )
    parser.add_argument("--profiles", default="multi_switch_complex,mutation_then_query,two_traversals")
    parser.add_argument("--seeds", default="11,22,33")
    parser.add_argument("--out-dir", default=str(DEFAULT_OUT_DIR))
    parser.add_argument("--out-csv", default=None)
    parser.add_argument("--omp-threads", type=int, default=None)
    args = parser.parse_args()

    out_dir = Path(args.out_dir)
    workload_root = out_dir / "workloads"
    log_root = out_dir / "logs"
    workload_root.mkdir(parents=True, exist_ok=True)
    log_root.mkdir(parents=True, exist_ok=True)

    out_csv = Path(args.out_csv) if args.out_csv else out_dir / "ablation_switch_cases.csv"
    rows: list[dict[str, object]] = []

    profiles = [p.strip() for p in args.profiles.split(",") if p.strip()]
    for profile in profiles:
        profile_dir = workload_root / profile
        graphs = ensure_workloads(profile_dir, args.seeds, profile)
        for graph_path in graphs:
            case_name = f"{profile}__{graph_path.stem}"
            try:
                switches, compile_text = compile_graph(graph_path)
            except subprocess.CalledProcessError as exc:
                text = (exc.stdout or "") + "\n" + (exc.stderr or "")
                (log_root / f"{case_name}.compile.log").write_text(text, encoding="utf-8")
                rows.append(
                    {
                        "Profile": profile,
                        "Graph": graph_path.name,
                        "Switches_Auto": 0,
                        "Time_Auto_ms": None,
                        "RSS_Auto_KB": None,
                        "Status": "compile_fail",
                    }
                )
                continue

            (log_root / f"{case_name}.compile.log").write_text(compile_text, encoding="utf-8")
            if switches <= 0:
                continue

            try:
                time_ms, rss_kb, run_text = time_run(graph_path, args.omp_threads)
                status = "ok"
            except (subprocess.CalledProcessError, RuntimeError) as exc:
                if isinstance(exc, subprocess.CalledProcessError):
                    run_text = (exc.stdout or "") + "\n" + (exc.stderr or "")
                else:
                    run_text = str(exc)
                time_ms = None
                rss_kb = None
                status = "run_fail"

            (log_root / f"{case_name}.run.log").write_text(run_text, encoding="utf-8")
            rows.append(
                {
                    "Profile": profile,
                    "Graph": graph_path.name,
                    "Switches_Auto": switches,
                    "Time_Auto_ms": time_ms,
                    "RSS_Auto_KB": rss_kb,
                    "Status": status,
                }
            )

    rows.sort(key=lambda row: (str(row["Profile"]), str(row["Graph"])))
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=["Profile", "Graph", "Switches_Auto", "Time_Auto_ms", "RSS_Auto_KB", "Status"],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"[done] wrote {out_csv}")
    print(f"[done] switch-positive cases: {sum(1 for row in rows if row['Switches_Auto'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
