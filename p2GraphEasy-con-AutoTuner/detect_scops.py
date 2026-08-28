#!/usr/bin/env python3
"""Parse `opt -polly-print-detect` output and summarize SCoPs per function."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class FunctionScops:
    name: str
    regions: list[str] = field(default_factory=list)


@dataclass
class ScopReport:
    functions: list[FunctionScops] = field(default_factory=list)

    @property
    def total(self) -> int:
        return sum(len(fn.regions) for fn in self.functions)

    def by_name(self, name: str) -> FunctionScops | None:
        for fn in self.functions:
            if fn.name == name:
                return fn
        return None


FUNCTION_HEADER = re.compile(
    r"Printing analysis 'Polly - Detect static control parts \(SCoPs\)' for function '([^']+)':"
)
VALID_REGION = re.compile(r"Valid Region for Scop:\s*(.+)")


def parse_polly_detect(text: str) -> ScopReport:
    report = ScopReport()
    current: FunctionScops | None = None

    for line in text.splitlines():
        header = FUNCTION_HEADER.search(line)
        if header:
            current = FunctionScops(name=header.group(1))
            report.functions.append(current)
            continue
        if current is None:
            continue
        region = VALID_REGION.search(line)
        if region:
            current.regions.append(region.group(1).strip())

    return report


def run_opt_detect(opt_bin: Path, bc_path: Path) -> str:
    cmd = [str(opt_bin), "-polly-print-detect", "-disable-output", str(bc_path)]
    proc = subprocess.run(cmd, check=False, capture_output=True, text=True)
    if proc.returncode not in (0, 74):  # 74 = analysis-only exit in some LLVM builds
        raise RuntimeError(proc.stderr or proc.stdout or f"opt failed: {proc.returncode}")
    return proc.stdout


def print_report(report: ScopReport, focus: str | None, verbose: bool) -> None:
    if focus:
        fn = report.by_name(focus)
        if fn is None:
            print(f"{focus}: 0 SCoP(s)")
            return
        print(f"{fn.name}: {len(fn.regions)} SCoP(s)")
        if verbose:
            for region in fn.regions:
                print(f"  {region}")
        return

    user_fns = [
        fn
        for fn in report.functions
        if not fn.name.startswith("_Z") and fn.name not in {"main"} or fn.name in {"main", "tripleLoop", "matmul3", "initMatrix"}
    ]
    # Print user-facing functions first, then rest with SCoPs.
    printed: set[str] = set()
    for fn in report.functions:
        if fn.name in {"main", "tripleLoop", "matmul3", "initMatrix"} or (
            not fn.name.startswith("_Z") and not fn.name.startswith("__")
        ):
            if fn.regions or fn.name in {"main", "tripleLoop", "matmul3"}:
                print(f"{fn.name}: {len(fn.regions)} SCoP(s)")
                if verbose and fn.regions:
                    for region in fn.regions:
                        print(f"  {region}")
                printed.add(fn.name)

    extras = [fn for fn in report.functions if fn.regions and fn.name not in printed]
    if extras and verbose:
        print("-- runtime / stdlib SCoPs --")
        for fn in extras:
            print(f"{fn.name}: {len(fn.regions)} SCoP(s)")

    print(f"module total: {report.total} SCoP(s)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bc", type=Path, help="LLVM bitcode file")
    parser.add_argument(
        "--opt",
        type=Path,
        default=Path("/usr/local/llvm-20-polly-rtti/bin/opt"),
        help="path to opt with Polly enabled",
    )
    parser.add_argument("--function", "-f", help="only report this LLVM function")
    parser.add_argument("--verbose", "-v", action="store_true")
    parser.add_argument("--json", action="store_true", help="machine-readable summary")
    args = parser.parse_args()

    text = run_opt_detect(args.opt, args.bc)
    report = parse_polly_detect(text)

    if args.json:
        import json

        payload = {
            "total": report.total,
            "functions": [
                {"name": fn.name, "scops": len(fn.regions), "regions": fn.regions}
                for fn in report.functions
                if fn.regions or fn.name == (args.function or fn.name)
            ],
        }
        if args.function:
            fn = report.by_name(args.function)
            payload = {
                "function": args.function,
                "scops": 0 if fn is None else len(fn.regions),
                "regions": [] if fn is None else fn.regions,
            }
        print(json.dumps(payload, indent=2))
        return 0

    print_report(report, args.function, args.verbose)
    return 0


if __name__ == "__main__":
    sys.exit(main())
