#!/usr/bin/env python3

import argparse
import heapq
import json
import math
from collections import deque
from pathlib import Path


ROOT = Path(__file__).resolve().parent
METADATA = ROOT / "graph_metadata.json"
INF = 2_147_483_647


def edge_weight(source: int, offset: int) -> int:
    return 1 + ((source * 17 + offset * 31) % 9)


def read_values(path: Path, converter):
    values = []
    with path.open() as stream:
        for line in stream:
            line = line.strip()
            if line:
                values.append(converter(line))
    return values


def neighbors(vertex: int, n: int, offsets: list[int]):
    for offset in offsets:
        yield (vertex + offset) % n
        yield (vertex - offset) % n


def weighted_neighbors(vertex: int, n: int, offsets: list[int]):
    for offset in offsets:
        yield (vertex + offset) % n, edge_weight(vertex, offset)
        source = (vertex - offset) % n
        yield source, edge_weight(source, offset)


def reference_bfs(n: int, offsets: list[int]) -> list[int]:
    distance = [-1] * n
    distance[0] = 0
    queue = deque([0])
    while queue:
        source = queue.popleft()
        next_distance = distance[source] + 1
        for destination in neighbors(source, n, offsets):
            if distance[destination] == -1:
                distance[destination] = next_distance
                queue.append(destination)
    return distance


def verify_bfs(path: Path, n: int, offsets: list[int]) -> None:
    parent = read_values(path, int)
    assert len(parent) == n, f"BFS emitted {len(parent)} values, expected {n}"
    distance = reference_bfs(n, offsets)
    offset_set = set(offsets)
    assert parent[0] == 0, f"BFS source parent is {parent[0]}, expected 0"
    for vertex in range(1, n):
        p = parent[vertex]
        assert 0 <= p < n, f"BFS parent[{vertex}]={p} is invalid"
        delta = (vertex - p) % n
        assert (
            delta in offset_set or (-delta) % n in offset_set
        ), f"BFS parent edge ({p}, {vertex}) does not exist"
        assert distance[p] + 1 == distance[vertex], (
            f"BFS parent[{vertex}]={p} is not on the preceding level"
        )


def verify_cc(path: Path, n: int) -> None:
    labels = read_values(path, int)
    assert len(labels) == n, f"CC emitted {len(labels)} values, expected {n}"
    assert all(label == 0 for label in labels), (
        f"CC produced {len(set(labels))} labels on a connected graph"
    )


def verify_kcore(path: Path, n: int, degree: int) -> None:
    alive = read_values(path, int)
    assert len(alive) == n, f"K-core emitted {len(alive)} values, expected {n}"
    assert degree >= 5
    assert all(value == 1 for value in alive), (
        f"5-core removed {sum(value != 1 for value in alive)} vertices "
        f"from a {degree}-regular graph"
    )


def verify_pagerank(path: Path, n: int) -> None:
    ranks = read_values(path, float)
    assert len(ranks) == n, (
        f"PageRank emitted {len(ranks)} values, expected {n}"
    )
    expected = 1.0 / n
    max_error = max(abs(rank - expected) for rank in ranks)
    assert max_error <= 2e-7, (
        f"PageRank maximum error {max_error:.3e} exceeds tolerance"
    )
    assert math.isclose(sum(ranks), 1.0, rel_tol=0.0, abs_tol=2e-3), (
        f"PageRank sum is {sum(ranks):.9f}, expected approximately 1"
    )


def reference_sssp(n: int, offsets: list[int]) -> list[int]:
    distance = [INF] * n
    distance[0] = 0
    queue = [(0, 0)]
    while queue:
        current, source = heapq.heappop(queue)
        if current != distance[source]:
            continue
        for destination, weight in weighted_neighbors(source, n, offsets):
            candidate = current + weight
            if candidate < distance[destination]:
                distance[destination] = candidate
                heapq.heappush(queue, (candidate, destination))
    return distance


def verify_sssp(path: Path, n: int, offsets: list[int]) -> None:
    actual = read_values(path, int)
    assert len(actual) == n, f"SSSP emitted {len(actual)} values, expected {n}"
    expected = reference_sssp(n, offsets)
    mismatches = [
        (vertex, actual[vertex], expected[vertex])
        for vertex in range(n)
        if actual[vertex] != expected[vertex]
    ]
    assert not mismatches, f"SSSP mismatches (first 10): {mismatches[:10]}"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--results",
        type=Path,
        default=ROOT / "results",
        help="directory containing one <algorithm>.out file per algorithm",
    )
    args = parser.parse_args()

    metadata = json.loads(METADATA.read_text())
    n = metadata["vertices"]
    offsets = metadata["offsets"]
    degree = metadata["undirected_degree"]

    checks = [
        ("bfs_parent", lambda p: verify_bfs(p, n, offsets)),
        ("cc", lambda p: verify_cc(p, n)),
        ("kcore", lambda p: verify_kcore(p, n, degree)),
        ("pagerank", lambda p: verify_pagerank(p, n)),
        ("sssp", lambda p: verify_sssp(p, n, offsets)),
    ]
    for name, check in checks:
        check(args.results / f"{name}.out")
        print(f"{name}: PASS")


if __name__ == "__main__":
    main()
