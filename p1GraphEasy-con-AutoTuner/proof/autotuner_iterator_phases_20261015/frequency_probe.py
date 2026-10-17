import sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from layout_switch_benchmark import build, run, environment
OUT = Path(__file__).resolve().parent / "frequency_probe"
work, objects = build(OUT)
for rounds in (1, 20):
    source = OUT / f"rounds{rounds}.sgpl"
    source.write_text(f'''graph G {{ edges: file "{ROOT / 'test/shared_edgelist.txt'}"; }};
int out_degree[6];
int round = 0;
while (round < {rounds}) {{
  for u in G.nodes {{
    for v in G.neighbors(u) {{
      out_degree[u] = out_degree[u] + 1;
    }}
  }}
  round = round + 1;
}}
print out_degree[1];
''')
    env = environment()
    env.update(AUTOTUNER_FORCE_LAYOUT="CSR", SGPL_FRONTIER_STRICT="1", GRAPH_DISABLE_POLLY="1", GRAPH_FRONTIER_STATS="1", FORCE_CPU="1", FORCE_GPU="0")
    for name, compiler in (("before", OUT.parent / "GraphProgram.before"), ("after", ROOT / "GraphProgram")):
        ir = OUT / f"{name}.rounds{rounds}.ll"
        result = run([compiler, source, "--ir-backend=cpu", f"--emit-ir-to={ir}"], work, env)
        (OUT / f"{name}.rounds{rounds}.log").write_text(result.stdout + result.stderr)
