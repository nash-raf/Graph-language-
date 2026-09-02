#!/usr/bin/env python3
import re
import subprocess
import sys
import tempfile
from pathlib import Path


LAYOUT_NAMES = {
    0: "CSR",
    1: "PCSR",
    2: "BCSR",
    3: "SET",
}


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: extract_autotuner_schedule.py <workload.graph> [expected-layout ...]", file=sys.stderr)
        return 2

    root = Path(__file__).resolve().parent.parent
    graph_path = Path(sys.argv[1])
    if not graph_path.is_absolute():
        graph_path = (Path.cwd() / graph_path).resolve()
    expected = sys.argv[2:]

    graph_program = root / "GraphProgram"
    if not graph_program.exists():
        print(f"missing compiler binary: {graph_program}", file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory(prefix="autotuner-ir-") as tmpdir:
        ir_path = Path(tmpdir) / "workload.ll"
        cmd = [str(graph_program), "--emit-ir-to", str(ir_path), str(graph_path)]
        result = subprocess.run(cmd, cwd=root, text=True, capture_output=True)
        if result.returncode != 0:
            sys.stderr.write(result.stdout)
            sys.stderr.write(result.stderr)
            return result.returncode

        text = ir_path.read_text()

    calls = []
    for line in text.splitlines():
        if "@autograph_ensure_layout(" not in line:
            continue
        match = re.search(r"i32 noundef (\d+)\)\s*$", line)
        if not match:
            match = re.search(r"i32 (\d+)\)\s*$", line)
        if match:
            calls.append(int(match.group(1)))

    names = [LAYOUT_NAMES.get(layout, f"UNKNOWN({layout})") for layout in calls]
    if not names:
        print(f"{graph_path.name}: no injected layout conversions")
    else:
        print(f"{graph_path.name}: {' -> '.join(names)}")

    if expected:
        if names != expected:
            print(f"expected: {' -> '.join(expected)}", file=sys.stderr)
            print(f"observed: {' -> '.join(names) if names else '(none)'}", file=sys.stderr)
            return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
