#!/usr/bin/env python3
"""Parse GRAPH_PARALLEL_DEBUG output: pick the loop site with the most cost-*
decisions, take its LAST cost line (post-calibration) plus the last c_state
seen for that site, and append one eta.csv row.

usage: parse_cost_dbg.py <dbg> <csv> <fixture> <graph> <N> <P> <rounds> <t_s> "<ans>"
"""
import re
import sys

dbg, csv, fixture, graph, N, P, rounds, t_s, ans = sys.argv[1:10]
cost = re.compile(
    r"cost-(doall|doacross) loop=(\S+) loop_id=(-?\d+) invocation=(\d+) choose=(\w+) "
    r"N=(\d+) P=(\d+) c_ns=([0-9.]+) L_thread_ns=([0-9.]+) L_path_ns=([0-9.]+) "
    r"L_total_ns=([0-9.]+).*?samples=(\d+)")
miss = re.compile(r"decision-cache-miss loop=(\S+) loop_id=(-?\d+).*?c_state=(\w+)")

by_name = {}
state_by_name = {}
for ln in open(dbg, errors="replace"):
    m = miss.search(ln)
    if m:
        state_by_name[m.group(1)] = m.group(3)
    m = cost.search(ln)
    if m:
        by_name.setdefault(m.group(2), []).append(m)

row = None
if by_name:
    name = max(by_name, key=lambda k: len(by_name[k]))
    m = by_name[name][-1]
    c_ns = float(m.group(8))
    thr_calc = ""
    if c_ns > 0 and int(P) > 1:
        thr_calc = f"{float(m.group(11)) / (c_ns * (1 - 1 / int(P))):.1f}"
    N_out = m.group(6) if m.group(6) not in ("", "0") else N   # the model prints the real trip count
    row = [fixture, graph, N_out, P, rounds, t_s, f"{c_ns:.3f}", m.group(9), m.group(10),
           m.group(11), f"{1 - 1 / int(P):.6f}", thr_calc or "-", m.group(12),
           state_by_name.get(name, "?"), m.group(5), ans, name, str(len(by_name[name]))]
else:
    row = [fixture, graph, N, P, rounds, t_s] + ["-"] * 9 + ["no-site", ans, "-", "0"]
with open(csv, "a") as fh:
    fh.write(",".join(row) + "\n")
print(",".join(row))
