#!/usr/bin/env python3
"""Check the new CSV against all measured default-backend observations."""
import csv
import hashlib
import json
import statistics
from benchmark_default_layout_applications import OUT, ROOT, MODES


def main():
    rows = list(csv.DictReader((OUT/'layout_application_comparison.csv').open()))
    raw = list(csv.DictReader((OUT/'validation_raw_runs.csv').open()))
    manifest = json.loads((OUT/'manifest.json').read_text())
    assert len(rows) == 5 and len(raw) == 115
    assert len({(r['case'],r['variant'],r['run']) for r in raw}) == 115
    graph_hash = hashlib.sha256((OUT/'graphs/community_projection.txt').read_bytes()).hexdigest()
    assert all(row['graph_sha256'] == graph_hash and row['compiler_backend'] == 'default' for row in rows)
    for name, digest in manifest['source_sha256'].items():
        assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest() == digest, name
    for row in rows:
        dest = OUT/'binaries'/row['case']
        r = json.loads((dest/'result.json').read_text())
        audit = json.loads((dest/'audit.json').read_text())
        assert r['settings'] == {} and r['warmups'] == 0
        saved = OUT/row['sgpl_file']
        assert saved.read_bytes() == (ROOT/'bench_results/layout_applications_20261015/winning_codes'/(r['kind']+'.sgpl')).read_bytes()
        for mode in MODES:
            selected = [x for x in raw if x['case'] == row['case'] and x['variant'] == mode]
            expected_count = 1 if mode in ('CSR','BCSR') else 7
            assert len(selected) == expected_count == int(row[mode+'_samples'])
            samples = [float(x['total_runtime_ms']) for x in selected]
            assert samples == r['samples_ms'][mode]
            assert abs(statistics.median(samples)-float(row[mode+'_total_ms'])) < 1e-9
            assert all(x['semantic_output_verified'] == 'True' for x in selected)
            assert audit[mode]['correct'] and audit[mode]['iterator_epochs_verified']
            assert audit[mode]['graph_fingerprints_verified'] and audit[mode]['layouts_checked_on_all_calls']
        assert row['AUTO_executed_layouts'] == 'CSR;SET;CSR'
        best = min(MODES[1:], key=lambda m:float(row[m+'_total_ms']))
        assert row['best_fixed_layout'] == best
        ratio = float(row[best+'_total_ms'])/float(row['AUTO_total_ms'])
        assert abs(ratio-float(row['speedup_vs_best_fixed'])) < 1e-12
        assert (row['autotuner_faster'] == 'true') == (ratio > 1)
    print('Validated 5 unchanged programs, 1 shared graph, 25 default-backend variants and 115 full timed launches.')


if __name__ == '__main__':
    main()
