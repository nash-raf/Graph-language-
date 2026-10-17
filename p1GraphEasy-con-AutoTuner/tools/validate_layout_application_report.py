#!/usr/bin/env python3
"""Check exported application report against its saved executable observations."""
import csv
import hashlib
import json
import statistics
from layout_application_cases import OUT

def main():
    raw=list(csv.DictReader((OUT/'validation_raw_runs.csv').open()))
    rows=list(csv.DictReader((OUT/'layout_application_wins.csv').open()))
    assert len(rows)==5 and len(raw)==107
    assert len({(r['case'],r['variant'],r['run']) for r in raw})==107
    modes=['AUTO','CSR','PCSR','BCSR','SET']
    graph_hash=hashlib.sha256((OUT/'graphs/community_projection.txt').read_bytes()).hexdigest()
    assert len({r['graph_sha256'] for r in rows})==1
    for row in rows:
        name=row['case'];saved=OUT/row['sgpl_file']
        result=json.loads((OUT/'binaries'/name/'result.json').read_text())
        audit=json.loads((OUT/'binaries'/name/'audit.json').read_text())
        original=OUT/'codes'/saved.name.replace('.sgpl','_events80000_batch8.sgpl')
        assert saved.read_bytes()==original.read_bytes()
        assert row['graph_sha256']==graph_hash
        assert result['settings']=={'GRAPH_FRONTIER_REWRITE_OFF':'1'}
        for mode in modes:
            selected=[r for r in raw if r['case']==name and r['variant']==mode]
            assert len(selected)==int(row[mode+'_samples'])==len(result['samples_ms'][mode])
            samples=[float(r['total_runtime_ms']) for r in selected]
            assert abs(statistics.median(samples)-float(row[mode+'_total_ms']))<1e-9
            assert all(r['semantic_output_verified']=='True' for r in selected)
            assert audit[mode]['correct'] and audit[mode]['layouts_checked_on_all_calls']
        ratio=min(float(row[m+'_total_ms']) for m in modes[1:])/float(row['AUTO_total_ms'])
        assert abs(ratio-float(row['speedup_vs_best_fixed']))<1e-12 and ratio>1
    print('Validated 5 distinct saved programs, 1 shared input, 25 audited variants, and 107 full timed launches.')

if __name__=='__main__':main()
