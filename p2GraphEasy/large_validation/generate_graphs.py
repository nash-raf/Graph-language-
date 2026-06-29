#!/usr/bin/env python3

import json
import random
from pathlib import Path


NUM_VERTICES = 100_000
NUM_OFFSETS = 100
SEED = 20_260_622
ROOT = Path(__file__).resolve().parent
UNWEIGHTED = ROOT / "synth_v100000_e10000000.txt"
WEIGHTED = ROOT / "synth_v100000_e10000000_w.txt"
METADATA = ROOT / "graph_metadata.json"


def edge_weight(source: int, offset: int) -> int:
    return 1 + ((source * 17 + offset * 31) % 9)


def choose_offsets() -> list[int]:
    rng = random.Random(SEED)
    return [1, *sorted(rng.sample(range(2, NUM_VERTICES // 2), NUM_OFFSETS - 1))]


def main() -> None:
    offsets = choose_offsets()
    edge_count = NUM_VERTICES * len(offsets)

    with UNWEIGHTED.open("w", buffering=1024 * 1024) as unweighted, \
            WEIGHTED.open("w", buffering=1024 * 1024) as weighted:
        for source in range(NUM_VERTICES):
            for offset in offsets:
                destination = (source + offset) % NUM_VERTICES
                unweighted.write(f"{source} {destination}\n")
                weighted.write(
                    f"{source} {destination} {edge_weight(source, offset)}\n"
                )

    METADATA.write_text(
        json.dumps(
            {
                "vertices": NUM_VERTICES,
                "input_edges": edge_count,
                "undirected_degree": 2 * len(offsets),
                "seed": SEED,
                "offsets": offsets,
            },
            indent=2,
        )
        + "\n"
    )

    print(f"vertices: {NUM_VERTICES}")
    print(f"input edges: {edge_count}")
    print(f"unweighted: {UNWEIGHTED}")
    print(f"weighted: {WEIGHTED}")


if __name__ == "__main__":
    main()
