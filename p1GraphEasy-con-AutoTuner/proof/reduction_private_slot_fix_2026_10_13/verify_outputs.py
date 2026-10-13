from pathlib import Path
import json

out = Path(__file__).resolve().parent
root = out.parent.parent
edges = [tuple(map(int, line.split())) for line in
         (root.parent / 'verify/fixtures/g20k.txt').read_text().splitlines() if line.strip()]
arc_count = 2 * len(edges)
vertex_ids = [vertex for edge in edges for vertex in edge]
expected = {
    'two_reduce_slots': ['a', '10', 'b', '20'],
    'priv_two_slots_big': ['a', str(len(edges)), 'b', str(sum(v for _, v in edges))],
    'reduce_real_ops': ['rsum', format(arc_count, 'g'), 'rmn', str(min(vertex_ids)),
                        'rmx', str(max(vertex_ids))],
}
results = {}
for name, wanted in expected.items():
    outputs = {}
    for suffix in ['serial', 't1.p1', 't4.p4', 't4.p3']:
        path = out / f'{name}.{suffix}.out'
        lines = [line for line in path.read_text().splitlines() if 'AutoTuner' not in line]
        actual = ' '.join(lines).split()
        assert actual == wanted, (name, suffix, actual, wanted)
        outputs[suffix] = ' '.join(actual)
    results[name] = {'expected': ' '.join(wanted), 'outputs': outputs}
    print(f'PASS {name}: all thread/partition configurations match serial and independent expected values')
(out / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
