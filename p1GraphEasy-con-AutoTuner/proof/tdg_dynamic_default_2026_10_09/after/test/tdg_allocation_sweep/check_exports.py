#!/usr/bin/env python3
"""Independently check saved sweep coverage, statistics, plots and source hashes."""
import argparse
import csv
import hashlib
import itertools
import json
from pathlib import Path
import statistics
from collections import defaultdict
from run import configurations, ROOT, verify_recorded_source

def read(path):
    with path.open() as f: return list(csv.DictReader(f))

def main():
    p=argparse.ArgumentParser(); p.add_argument('out',type=Path); args=p.parse_args()
    out=args.out.resolve(); manifest=json.loads((out/'manifest.json').read_text())
    raw=read(out/'raw.csv'); summaries=read(out/'summary.csv'); allocations=read(out/'allocations.csv')
    schedules=read(out/'schedules.csv'); groups=defaultdict(list)
    for r in raw: groups[r['case'],int(r['budget']),r['policy']].append(r)
    repeats=manifest['arguments']['repeats']; samples=manifest['arguments']['samples']
    total_schedules=0
    for case,budget,policy in itertools.product(manifest['cases'],manifest['arguments']['budgets'],('ordinary-first','dynamic')):
        key=case['name'],budget,policy; rows=groups[key]
        expected={':'.join(map(str,c)) for c in configurations(budget,case['n'],policy,manifest.get('oracle_version',1)>=2)}
        total_schedules+=len(expected)
        assert len(rows)==repeats*samples*(len(expected)+1),key
        for rep in range(repeats):
            block=[r for r in rows if int(r['repeat'])==rep]
            assert len([r for r in block if r['kind']=='model'])==samples
            forced=[r for r in block if r['kind']=='forced']
            assert set(r['config'] for r in forced)==expected
            for r in forced:
                c=tuple(map(int,r['config'].split(':')))
                observed=tuple(int(r[k]) for k in ('backend','actual_left','actual_right','order','parents'))
                assert c==observed
        exported=[r for r in allocations if (r['case'],int(r['budget']),r['policy'])==key]
        assert {(int(r['left']),int(r['right'])) for r in exported}=={
            tuple(map(int,c.split(':')[1:3])) for c in expected}
        exported_schedules=[r for r in schedules if (r['case'],int(r['budget']),r['policy'])==key]
        assert len(exported_schedules)==len(expected)
        summary=next(r for r in summaries if (r['case'],int(r['budget']),r['policy'])==key)
        native=[statistics.median(int(r['time_ns']) for r in rows if r['kind']=='model' and int(r['repeat'])==rep)
                for rep in range(repeats)]
        best=min(float(r['median_ns']) for r in exported_schedules)
        assert abs(float(summary['model_ms'])-statistics.median(native)/1e6)<1e-10
        assert abs(float(summary['best_ms'])-best/1e6)<1e-10
        assert abs(float(summary['model_over_best_ratio'])-statistics.median(native)/best)<1e-10
        stem=f'{key[0]}.B{budget}.{policy}'
        for suffix in ('.png','.svg'): assert (out/'plots'/(stem+suffix)).stat().st_size>5000
        assert '<h2>'+stem+'</h2>' in (out/'tables.html').read_text()
    original_names={case['name'] for case in manifest['cases']}
    assert len([key for key in groups if key[0] in original_names])==len(manifest['cases'])*len(manifest['arguments']['budgets'])*2
    raw_count=len(raw); observed_names={key[0] for key in groups}
    if (out/'mixed_manifest.json').exists():
        # Release the million-row combined dataset before the mixed verifier
        # reads it; do not keep two complete dictionary copies in memory.
        del raw,groups
        from mixed import check
        check(out)
        mixed_names={case['name'] for case in json.loads((out/'mixed_manifest.json').read_text())['cases']}
        assert observed_names==original_names|mixed_names
    else:
        assert len(groups)==len(manifest['cases'])*len(manifest['arguments']['budgets'])*2
    for name,digest in manifest['protected_sha256'].items():
        verify_recorded_source(name,digest)
    for record in manifest['generated']:
        assert hashlib.sha256((out/'build'/(record['case']+'.o')).read_bytes()).hexdigest()==record['object_sha256']
    message=(f'PASS: {raw_count} CSV timing references (shared mixed oracle rows are counted twice); {total_schedules} complete original allocation/schedule configurations; '
             f'{len(summaries)} case/budget/policy results.\n'
             'PASS: actual grants, CSV statistics, every allocation bar/table, generated objects and protected source hashes.\n')
    (out/'export_validation.txt').write_text(message); print(message,end='')

if __name__=='__main__': main()
