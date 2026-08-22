#!/usr/bin/env python3
"""DOACROSS regression after store-forwarded PHI fix + Polly/DOALL combo."""

from __future__ import annotations

import csv
import os
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LLVM = Path(os.environ.get("LLVM_PREFIX", "/usr/local/llvm-20-polly-rtti"))
LLVM_DIS = LLVM / "bin" / "llvm-dis"
OUT = ROOT / "benchmark_logs" / f"doacross_fix_{time.strftime('%Y%m%d_%H%M%S')}"
N = int(os.environ.get("N", "48"))


def header(edges: str) -> str:
    return f'graph G {{\n  edges: file "{edges}";\n  TRUE\n}};\n\n'


def write_edges(path: Path, n: int) -> None:
    path.write_text("".join(f"{i} {i+1} 1\n" for i in range(max(1, n - 1))))


# --- 10 focused kernels ---
CASES: dict[str, tuple[str, str]] = {}


def add(name: str, expect: str, body: str) -> None:
    """expect: doacross | doall | sequential | combo_all3"""
    CASES[name] = (expect, body)


add(
    "01_prefix_pure",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) { a[i] = i + 1; i = i + 1; }
  i = 1;
  while (i < n) { a[i] = a[i] + a[i - 1]; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "02_prefix_plus_const",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) { a[i] = 1; i = i + 1; }
  i = 1;
  while (i < n) { a[i] = a[i - 1] + 3; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "03_prefix_plus_x_i",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int x[n];
  int i = 0;
  while (i < n) { a[i] = 0; x[i] = i + 1; i = i + 1; }
  i = 1;
  while (i < n) { a[i] = a[i - 1] + x[i]; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "04_prefix_indirect",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int x[n];
  int Q[n];
  int i = 0;
  while (i < n) { a[i] = 1; x[i] = i + 1; Q[i] = n - 1 - i; i = i + 1; }
  i = 1;
  while (i < n) {
    int idx = Q[i];
    a[i] = a[i - 1] + x[idx];
    i = i + 1;
  }
  print a[n - 1];
}
kernel();
""",
)

add(
    "05_running_mul",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) { a[i] = 2; i = i + 1; }
  i = 1;
  while (i < n) { a[i] = a[i - 1] * 2; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "06_saxpy_doall",
    "doall",
    """
fn void kernel() {
  int n = numVertices(G);
  int x[n];
  int y[n];
  int i = 0;
  while (i < n) { x[i] = i + 1; y[i] = 1; i = i + 1; }
  i = 0;
  while (i < n) { y[i] = 3 * x[i] + y[i]; i = i + 1; }
  print y[n - 1];
}
kernel();
""",
)

add(
    "07_independent_init",
    "doall",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) { a[i] = i * 2; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "08_fib_like",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int i = 0;
  while (i < n) { a[i] = 0; i = i + 1; }
  a[0] = 1;
  if (n > 1) { a[1] = 1; }
  i = 2;
  while (i < n) { a[i] = a[i - 1] + a[i - 2]; i = i + 1; }
  print a[n - 1];
}
kernel();
""",
)

add(
    "09_prefix_then_saxpy",
    "doacross",
    """
fn void kernel() {
  int n = numVertices(G);
  int a[n];
  int y[n];
  int i = 0;
  while (i < n) { a[i] = 1; y[i] = 0; i = i + 1; }
  i = 1;
  while (i < n) { a[i] = a[i - 1] + 1; i = i + 1; }
  i = 0;
  while (i < n) { y[i] = a[i] + y[i]; i = i + 1; }
  print y[n - 1];
}
kernel();
""",
)

add(
    "10_triple_polly_doall_doacross",
    "combo_all3",
    """
fn void kernel() {
  int n = numVertices(G);
  int A[n][n];
  int Bmat[n][n];
  int C[n][n];
  int x[n];
  int y[n];
  int pref[n];
  int Q[n];
  int i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      A[i][j] = i + 1;
      Bmat[i][j] = j + 1;
      C[i][j] = 0;
      j = j + 1;
    }
    x[i] = i + 1;
    y[i] = 1;
    pref[i] = 1;
    Q[i] = n - 1 - i;
    i = i + 1;
  }
  i = 0;
  while (i < n) {
    int j = 0;
    while (j < n) {
      int s = 0;
      int k = 0;
      while (k < n) {
        s = s + A[i][k] * Bmat[k][j];
        k = k + 1;
      }
      C[i][j] = s;
      j = j + 1;
    }
    i = i + 1;
  }
  i = 0;
  while (i < n) { y[i] = 3 * x[i] + y[i]; i = i + 1; }
  i = 1;
  while (i < n) {
    int idx = Q[i];
    pref[i] = pref[i - 1] + x[idx];
    i = i + 1;
  }
  print y[n - 1] + pref[n - 1] + C[1][1];
}
kernel();
""",
)


def analyze(bc: Path) -> dict:
    d = {
        "doall": 0,
        "doacross": 0,
        "seq": 0,
        "gomp": 0,
        "pfr": 0,
        "pfr_doall": 0,
        "pfr_doacross": 0,
        "polly": 0,
        "wait": 0,
    }
    if not bc.exists():
        return d
    ll = bc.with_suffix(".ll")
    subprocess.run([str(LLVM_DIS), str(bc), "-o", str(ll)], capture_output=True)
    text = ll.read_text(errors="replace") if ll.exists() else ""
    d["doall"] = text.count("parallel.type=DOALL")
    d["doacross"] = text.count("parallel.type=DOACROSS")
    d["seq"] = text.count("parallel.type=SEQUENTIAL")
    d["gomp"] = text.count("GOMP_parallel")
    d["pfr"] = len(re.findall(r"call void @parallel_for_runtime\(", text))
    d["pfr_doall"] = len(
        re.findall(r"call void @parallel_for_runtime\([^)]*, i32 0, i32 0\)", text)
    )
    d["pfr_doacross"] = len(
        re.findall(r"call void @parallel_for_runtime\([^)]*, i32 1, i32 1\)", text)
    )
    d["polly"] = len(re.findall(r"\bpolly\.", text))
    d["wait"] = text.count("doacross.wait")
    return d


def verdict(expect: str, pdg: dict, after: dict, post: dict, polly_on: bool) -> tuple[bool, str]:
    if expect == "doacross":
        ok = pdg["doacross"] > 0 or after["pfr_doacross"] > 0
        # With Polly on, must still see DOACROSS (the bug we fixed)
        if polly_on:
            ok = pdg["doacross"] > 0 or after["pfr_doacross"] > 0
        return ok, f"doacross md={pdg['doacross']} pfr11={after['pfr_doacross']}"
    if expect == "doall":
        ok = pdg["doall"] > 0 and pdg["doacross"] == 0
        return ok, f"doall md={pdg['doall']} doacross md={pdg['doacross']}"
    if expect == "combo_all3":
        if not polly_on:
            # off: need doall+doacross at least
            ok = pdg["doall"] > 0 and (pdg["doacross"] > 0 or after["pfr_doacross"] > 0)
            return ok, f"off doall={pdg['doall']} doacross={pdg['doacross']}"
        ok = (
            post["gomp"] > 0
            and (pdg["doall"] > 0 or after["pfr_doall"] > 0)
            and (pdg["doacross"] > 0 or after["pfr_doacross"] > 0)
        )
        return ok, (
            f"gomp={post['gomp']} doall={pdg['doall']}/{after['pfr_doall']} "
            f"doacross={pdg['doacross']}/{after['pfr_doacross']}"
        )
    return False, "unknown expect"


def run_case(name: str, expect: str, body: str, polly_on: bool) -> dict:
    tag = "polly_on" if polly_on else "polly_off"
    case = OUT / name / tag
    case.mkdir(parents=True, exist_ok=True)
    edges = case / "edges.txt"
    write_edges(edges, N)
    graph = case / f"{name}.graph"
    graph.write_text(header(str(edges)) + body)

    env = os.environ.copy()
    env["PATH"] = f"{LLVM}/bin:" + env.get("PATH", "")
    env.pop("GRAPH_DISABLE_POLLY", None)
    env.pop("GRAPH_POLLY_NO_PARALLEL", None)
    if not polly_on:
        env["GRAPH_DISABLE_POLLY"] = "1"
    post = case / "post.bc"
    pdg = case / "pdg.bc"
    after = case / "after.bc"
    env["DUMP_LLVM_BC_POST"] = str(post)
    env["DUMP_LLVM_BC_PDG"] = str(pdg)
    env["DUMP_LLVM_BC_AFTER_OUTLINE"] = str(after)

    proc = subprocess.run(
        [str(ROOT / "GraphProgram"), str(graph)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
    )
    (case / "log.txt").write_text(proc.stdout + "\n---\n" + proc.stderr)
    dumps_ok = pdg.exists()
    post_a = analyze(post)
    pdg_a = analyze(pdg)
    after_a = analyze(after)
    ok, detail = verdict(expect, pdg_a, after_a, post_a, polly_on) if dumps_ok else (False, "no dumps")
    return {
        "case": name,
        "cfg": tag,
        "expect": expect,
        "ok": ok,
        "detail": detail,
        "compile_rc": proc.returncode,
        "pdg_doall": pdg_a["doall"],
        "pdg_doacross": pdg_a["doacross"],
        "pdg_seq": pdg_a["seq"],
        "gomp": post_a["gomp"],
        "pfr_doall": after_a["pfr_doall"],
        "pfr_doacross": after_a["pfr_doacross"],
        "doacross_wait": pdg_a["wait"] + after_a["wait"],
    }


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    if not (ROOT / "GraphProgram").exists():
        print("GraphProgram missing", file=sys.stderr)
        return 1
    # ensure bfs stub
    stub = ROOT / "bfs_runtime.ll"
    if not stub.exists():
        stub.write_text(
            '; ModuleID = "stub"\nsource_filename = "stub.c"\n'
            'target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"\n'
            'target triple = "x86_64-unknown-linux-gnu"\n'
        )

    rows = []
    print(f"OUT={OUT} N={N}")
    for name, (expect, body) in CASES.items():
        for polly_on in (False, True):
            # combo_all3 is the critical polly_on check; still run both
            print(f"  {name:32s} polly={'on' if polly_on else 'off':3s} ...", flush=True)
            row = run_case(name, expect, body, polly_on)
            rows.append(row)
            status = "PASS" if row["ok"] else "FAIL"
            print(
                f"    {status}  doacross={row['pdg_doacross']} doall={row['pdg_doall']} "
                f"gomp={row['gomp']} pfr11={row['pfr_doacross']} pfr00={row['pfr_doall']}  "
                f"({row['detail']})"
            )

    csv_path = OUT / "summary.csv"
    with csv_path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)

    # Focus report
    md = OUT / "REPORT.md"
    lines = [
        "# DOACROSS store-forwarded PHI fix — test report",
        "",
        f"N={N}",
        "",
        "| case | cfg | expect | ok | DOACROSS | DOALL | GOMP | pfr DOACROSS | pfr DOALL |",
        "|---|---|---|---|---:|---:|---:|---:|---:|",
    ]
    for r in rows:
        lines.append(
            f"| {r['case']} | {r['cfg']} | {r['expect']} | {'PASS' if r['ok'] else 'FAIL'} | "
            f"{r['pdg_doacross']} | {r['pdg_doall']} | {r['gomp']} | "
            f"{r['pfr_doacross']} | {r['pfr_doall']} |"
        )
    passed = sum(1 for r in rows if r["ok"])
    lines += ["", f"**{passed}/{len(rows)} passed**", ""]
    # Highlight combo
    combo = [r for r in rows if r["case"] == "10_triple_polly_doall_doacross"]
    lines.append("## Combo (Polly + DOALL + DOACROSS)")
    for r in combo:
        lines.append(f"- `{r['cfg']}`: {'PASS' if r['ok'] else 'FAIL'} — {r['detail']}")
    md.write_text("\n".join(lines))
    print(f"\n{passed}/{len(rows)} passed")
    print(f"Wrote {csv_path}\nWrote {md}")
    return 0 if passed == len(rows) else 1


if __name__ == "__main__":
    raise SystemExit(main())
