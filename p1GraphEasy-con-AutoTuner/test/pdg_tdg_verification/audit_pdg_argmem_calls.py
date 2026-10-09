#!/usr/bin/env python3
"""Inventory argument-memory-only calls in compiler IR immediately before PDG.

Usage: python3 test/pdg_tdg_verification/audit_pdg_argmem_calls.py COMPILER [FIXTURE ...]
Requires llvm-dis-20 (or LLVM_DIS).  With no fixtures, scans repository .graph
files smaller than 2 MiB.  Each compilation writes program.o in the repository.
"""

import collections
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[2]
ATTR = re.compile(r"^attributes #(\d+) = \{(.*)\}$", re.MULTILINE)
CALLEE = re.compile(r"@([A-Za-z0-9_.$-]+)\s*\(")
GROUP = re.compile(r"#(\d+)\b")
ARGMEM = re.compile(r"\bargmemonly\b|\bmemory\([^)]*\bargmem\s*:")


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2
    compiler = Path(sys.argv[1]).resolve()
    fixtures = ([Path(x).resolve() for x in sys.argv[2:]] or
                sorted(p for p in ROOT.rglob("*.graph")
                       if p.stat().st_size < 2_000_000))
    observed = collections.defaultdict(set)
    examples = {}
    failures = []
    with tempfile.TemporaryDirectory(prefix="sgpl-argmem-") as tmp:
        dump = Path(tmp) / "pre_pdg.bc"
        for index, fixture in enumerate(fixtures, 1):
            if index % 25 == 0:
                print(f"scanned_progress={index}/{len(fixtures)}", file=sys.stderr,
                      flush=True)
            dump.unlink(missing_ok=True)
            env = dict(os.environ, DUMP_LLVM_BC_PRE_PDG=str(dump))
            compile_note = ""
            try:
                run = subprocess.run(
                    [str(compiler), "--ir-backend=cpu", str(fixture)],
                    cwd=ROOT, env=env, capture_output=True, timeout=30)
            except subprocess.TimeoutExpired:
                run = None
                compile_note = "compiler timed out after pre-PDG dump"
            if not dump.exists():
                if run is None:
                    failures.append((fixture.name, "timeout before pre-PDG dump"))
                else:
                    failures.append((fixture.name, f"compile exit {run.returncode}: " +
                                     run.stderr.decode(errors="replace").strip().split("\n")[-1]))
                continue
            if compile_note:
                print(f"NOTE {fixture.name}: {compile_note}", file=sys.stderr)
            ir = subprocess.check_output(
                [os.environ.get("LLVM_DIS", "llvm-dis-20"), "-o", "-", str(dump)],
                text=True)
            attrs = {int(n): value for n, value in ATTR.findall(ir)}
            function_attrs = {}
            for line in ir.splitlines():
                if not line.startswith(("declare ", "define ")):
                    continue
                m = CALLEE.search(line)
                if m:
                    ids = [int(x) for x in GROUP.findall(line)]
                    function_attrs[m.group(1)] = line + " " + " ".join(
                        attrs.get(i, "") for i in ids)
            for line in ir.splitlines():
                if not re.search(r"\b(call|invoke|callbr)\b", line):
                    continue
                m = CALLEE.search(line)
                if not m:
                    continue
                name = m.group(1)
                ids = [int(x) for x in GROUP.findall(line)]
                effective = line + " " + function_attrs.get(name, "") + " " + \
                    " ".join(attrs.get(i, "") for i in ids)
                if ARGMEM.search(effective):
                    observed[name].add(fixture.name)
                    examples.setdefault(name, line.strip())
    print(f"scanned={len(fixtures)} failures={len(failures)}")
    for name, seen in sorted(observed.items()):
        sample_names = ",".join(sorted(seen)[:5])
        print(f"{name}\tfixtures={len(seen)}\tfirst={sample_names}"
              f"\texample={examples[name]}")
    for name, reason in failures:
        print(f"FAILED {name}: {reason}", file=sys.stderr)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
