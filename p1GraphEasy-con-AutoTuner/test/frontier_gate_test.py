#!/usr/bin/env python3
"""Check the AST motif gate before testing any runtime performance."""
from pathlib import Path
import os
import subprocess
import tempfile

repo = Path(__file__).resolve().parent.parent
algorithms = repo.parent / "algo"
with tempfile.TemporaryDirectory(prefix="sgpl-frontier-gate-") as temporary:
    logs = Path(temporary)
    for algorithm in ["bfs_parent", "cc", "kcore", "sssp"]:
        for legacy, backend in [(False, "cpu"), (True, "cpu"), (False, "gpu")]:
            env = dict(os.environ)
            for name in ["GRAPH_FRONTIER_REWRITE_OFF", "GRAPH_DISABLE_PDG", "FORCE_GPU"]:
                env.pop(name, None)
            env.update(FORCE_CPU="0" if backend == "gpu" else "1",
                       FORCE_GPU="1" if backend == "gpu" else "0", GRAPH_FRONTIER_STATS="1")
            if legacy:
                env["GRAPH_FRONTIER_REWRITE_OFF"] = "1"
            tag = algorithm + (".legacy" if legacy else ".default") + "." + backend
            result = subprocess.run([str(repo / "GraphProgram"), str(algorithms / (algorithm + ".graph"))],
                                    cwd=repo, env=env, text=True, capture_output=True)
            (logs / (tag + ".log")).write_text(result.stdout + result.stderr)
            if result.returncode:
                raise RuntimeError(tag + " compilation failed:\n" + result.stderr)
            undefined = subprocess.check_output(["nm", "-u", str(repo / "program.o")], text=True)
            motif = any(line.split()[-1] == "autograph_edgemap" for line in undefined.splitlines())
            if motif != legacy:
                raise AssertionError(f"{tag}: motif call={motif}, expected={legacy}\n" + result.stderr)
            if not legacy and "[spatial]" not in result.stderr:
                raise AssertionError(tag + ": original loops did not reach spatial analysis\n" + result.stderr)
            if not legacy and not any("group=Destination" in line and "runtime=1 realized=1" in line
                                      for line in result.stderr.splitlines() if line.startswith("[spatial]")):
                raise AssertionError(tag + ": automatic frontier partition was not realized\n" + result.stderr)
            print(f"PASS {tag}: autograph_edgemap={motif}", flush=True)
print("Frontier motif gate: PASS")
