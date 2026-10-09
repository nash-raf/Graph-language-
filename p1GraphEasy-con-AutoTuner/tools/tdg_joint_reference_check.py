#!/usr/bin/env python3
"""Compare the dynamic default with the frozen ordinary-first reference."""
import csv
import math
import random
import statistics
import subprocess
from tdg_joint_build import BUILD, OUT
from tdg_joint_benchmark import cases, environment


def main():
    rng = random.Random(20261008)
    rows = []
    for budget, name, args in cases():
        if budget != 4 or name not in ('fits-mixed', 'overflow-short', 'overflow-one-long',
                                       'overflow-all-long', 'tiny', 'streaming'):
            continue
        for repeat in range(3):
            order = ['reference', 'current']
            rng.shuffle(order)
            for policy in order:
                output = subprocess.check_output([str(BUILD/f'probe_{policy}'), *map(str, args)],
                    env=environment(budget, policy, False), text=True).strip().split(',')
                if policy=='reference' and (name.startswith('overflow') or name in ('tiny', 'streaming')):
                    assert output[11] == '0', (name, policy, output)
                rows.append(dict(case=name, run=repeat, policy=policy, steady_ns=float(output[7]),
                                 early_loops=int(output[11])))
    with (OUT/'current_reference.csv').open('w', newline='') as out:
        writer = csv.DictWriter(out, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    ratios = []
    for name in sorted({r['case'] for r in rows}):
        selected = [r for r in rows if r['case'] == name]
        for repeat in range(3):
            pair = {r['policy']:r['steady_ns'] for r in selected if r['run'] == repeat}
            ratios.append(pair['current']/pair['reference'])
    logs = [math.log(v) for v in ratios]
    boot = sorted(math.exp(statistics.mean(rng.choices(logs, k=len(logs)))) for _ in range(10000))
    text = (f'PASS frozen reference phase ordering and both runtimes correctness/budget assertions.\n'
            f'Default/frozen steady geometric ratio={math.exp(statistics.mean(logs)):.4f}; '
            f'paired timing-noise bootstrap 95% CI=[{boot[249]:.4f}, {boot[9749]:.4f}].\n')
    (OUT/'current_reference.txt').write_text(text)
    print(text)


if __name__ == '__main__':
    main()
