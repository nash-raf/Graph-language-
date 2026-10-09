#!/usr/bin/env python3
"""Independently check TDG's emitted-level trace against whole-task effects.

This checks the scheduler's decision, assuming the compiler's exported task
effects are complete.  Effect coverage is a separate proof obligation.
"""

import argparse
from pathlib import Path
import re
import sys

TASK = re.compile(r"\[tdg\.task\] id=(\d+) kind=\d+ graph=(\d+) opaque=(\d+)")
ACCESS = re.compile(r"\[tdg\.access\] task=(\d+) mode=([RW]) graph=\d+ object=(.*)")
EDGE = re.compile(r"\[tdg\.edge\] from=(\d+) to=(\d+) type=")
EMITTED = re.compile(r"\[tdg\.emitted\] level=(\d+) count=(\d+)")
MEMBER = re.compile(r"\[tdg\.emitted\.task\] level=(\d+) task=(\d+)")


def check_trace(data: str, forbid_graph_sharing: bool = False) -> list[str]:
    tasks: dict[int, dict] = {}
    edges: set[tuple[int, int]] = set()
    counts: dict[int, int] = {}
    groups: dict[int, list[int]] = {}
    errors: list[str] = []

    for line in data.splitlines():
        if match := TASK.search(line):
            task, graph, opaque = map(int, match.groups())
            tasks[task] = {"graph": bool(graph), "opaque": bool(opaque), "accesses": []}
        elif match := ACCESS.search(line):
            task, mode, obj = match.groups()
            task = int(task)
            if task not in tasks:
                errors.append(f"access for unknown task {task}")
            else:
                tasks[task]["accesses"].append((mode == "W", obj.strip()))
        elif match := EDGE.search(line):
            edges.add(tuple(map(int, match.groups())))
        elif match := EMITTED.search(line):
            level, count = map(int, match.groups())
            if level in counts:
                errors.append(f"duplicate emitted level {level}")
            counts[level] = count
        elif match := MEMBER.search(line):
            level, task = map(int, match.groups())
            groups.setdefault(level, []).append(task)

    if not tasks:
        return errors + ["no task summaries in trace"]
    successors: dict[int, set[int]] = {}
    for source, target in edges:
        successors.setdefault(source, set()).add(target)
    reachable: set[tuple[int, int]] = set()
    for source in tasks:
        pending = list(successors.get(source, ()))
        seen: set[int] = set()
        while pending:
            target = pending.pop()
            if target in seen:
                continue
            seen.add(target)
            reachable.add((source, target))
            pending.extend(successors.get(target, ()))
    for level, count in counts.items():
        members = groups.get(level, [])
        if len(members) != count:
            errors.append(f"level {level}: descriptor count {count}, logged members {len(members)}")
        if len(set(members)) != len(members):
            errors.append(f"level {level}: duplicate task")
        for i, left in enumerate(members):
            if left not in tasks:
                errors.append(f"level {level}: unknown task {left}")
                continue
            for right in members[i + 1:]:
                if right not in tasks:
                    errors.append(f"level {level}: unknown task {right}")
                    continue
                if (left, right) in reachable or (right, left) in reachable:
                    errors.append(f"level {level}: dependency path between tasks {left} and {right}")
                if forbid_graph_sharing and (tasks[left]["graph"] or tasks[right]["graph"]):
                    errors.append(f"level {level}: graph task shares a default level")
                if tasks[left]["opaque"] or tasks[right]["opaque"]:
                    errors.append(f"level {level}: opaque tasks {left} and {right} overlap")
                for left_write, left_obj in tasks[left]["accesses"]:
                    for right_write, right_obj in tasks[right]["accesses"]:
                        if (left_write or right_write) and (
                            not left_obj or not right_obj or left_obj == right_obj
                        ):
                            errors.append(
                                f"level {level}: conflicting tasks {left} and {right} "
                                f"on {left_obj or '?'} / {right_obj or '?'}"
                            )
                            break
                    else:
                        continue
                    break
    for level in groups.keys() - counts.keys():
        errors.append(f"level {level}: members without emitted-level record")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("traces", nargs="*", type=Path)
    parser.add_argument("--forbid-graph-sharing", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        mutant = "\n".join([
            "[tdg.task] id=0 kind=1 graph=1 opaque=0 accesses=1 blocks=2",
            "[tdg.access] task=0 mode=R graph=0 object=global:bias",
            "[tdg.task] id=1 kind=1 graph=0 opaque=0 accesses=1 blocks=2",
            "[tdg.access] task=1 mode=W graph=0 object=global:bias",
            "[tdg.emitted] level=0 count=2",
            "[tdg.emitted.task] level=0 task=0",
            "[tdg.emitted.task] level=0 task=1",
        ])
        assert check_trace(mutant), "checker accepted a conflicting level"
        print("PASS checker rejects a conflicting level")
        path_mutant = "\n".join([
            "[tdg.task] id=0 kind=1 graph=1 opaque=0 accesses=0 blocks=2",
            "[tdg.task] id=1 kind=0 graph=0 opaque=0 accesses=0 blocks=1",
            "[tdg.task] id=2 kind=1 graph=1 opaque=0 accesses=0 blocks=2",
            "[tdg.edge] from=0 to=1 type=SSA",
            "[tdg.edge] from=1 to=2 type=SSA",
            "[tdg.emitted] level=0 count=2",
            "[tdg.emitted.task] level=0 task=0",
            "[tdg.emitted.task] level=0 task=2",
        ])
        assert check_trace(path_mutant), "checker accepted a transitive dependency"
        print("PASS checker rejects a transitive dependency")
    for trace in args.traces:
        errors = check_trace(trace.read_text(errors="replace"), args.forbid_graph_sharing)
        if errors:
            for error in errors:
                print(f"{trace}: {error}", file=sys.stderr)
            return 1
        print(f"PASS {trace}: every emitted level satisfies its task summaries")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
