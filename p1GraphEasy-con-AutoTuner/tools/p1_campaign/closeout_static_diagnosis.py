"""Check complete component evidence and tabulate replicated effects."""
from pathlib import Path
import argparse
import collections
import html
import json
import statistics


def read(path):
    return json.loads(Path(path).read_text())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('evidence')
    parser.add_argument('output')
    args = parser.parse_args()
    root = Path(args.evidence) / 'p1-campaign/results'
    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=True)
    protocol = read(root / 'diagnostic_protocol.json')
    data = read(root / 'component_summary.json')
    records = read(root / 'components.json')
    assert len(protocol['selected']) == 22
    assert len(records) == 14080 and not any(x.get('error') for x in records)
    assert len(data['cases']) == 88
    counts = collections.Counter((x['algorithm'], x['graph'], x['workers'],
                                  x['independent_block'], x['label']) for x in records)
    assert len(counts) == 704 and set(counts.values()) == {20}
    rows = data['cases']
    tests = data['tests']
    aa = data['aa_tests']
    failures = [t for t in aa if t['holm_p'] < .05]
    stable = {(w, b): not any(t['workers'] == w and t['block'] == b and t['holm_p'] < .05
                              for t in aa) for w in [1, 2] for b in [0, 1]}
    findings = []
    for item in protocol['selected']:
        a, graph = item['algorithm'], item['graph']
        result = dict(item, widths={})
        for w in [1, 2]:
            selected = sorted([x for x in rows if x['algorithm'] == a and x['graph'] == graph
                               and x['workers'] == w], key=lambda x: x['block'])
            assert len(selected) == 2
            def contrast(left, right):
                return sorted([t for t in tests if t['algorithm'] == a and t['graph'] == graph
                               and t['workers'] == w and t['left'] == left and t['right'] == right],
                              key=lambda t: t['block'])
            full = contrast('accepted', 'candidate')
            layout = contrast('accepted', 'layout_only')
            dispatch = contrast('api_only', 'candidate')
            outlined = contrast('candidate', 'outlined')
            result['widths'][w] = dict(
                replicated_full_regression=all(stable[(w, t['block'])] and t['holm_p'] < .05
                                               and t['geometric_percent'] > 0 for t in full),
                context_effect_confirmed=all(stable[(w, t['block'])] and t['holm_p'] < .05
                                             and t['geometric_percent'] > 0 for t in layout),
                dispatcher_effect_confirmed=all(stable[(w, t['block'])] and t['holm_p'] < .05
                                                and t['geometric_percent'] > 0 for t in dispatch),
                outlining_recovery_confirmed=all(stable[(w, t['block'])] and t['holm_p'] < .05
                                                and t['geometric_percent'] < 0 for t in outlined),
                blocks=selected, full_tests=full, layout_tests=layout,
                dispatch_tests=dispatch, outline_tests=outlined)
        findings.append(result)
    profiles = read(root / 'component_profiles.json')
    assert len(profiles) == 200 and not any(x.get('error') for x in profiles)
    profile_groups = collections.defaultdict(list)
    for x in profiles:
        profile_groups[(x['algorithm'], x['graph'], x['workers'], x['label'])].append(x)
    prof = []
    for (a, graph, w, label), xs in sorted(profile_groups.items()):
        assert len(xs) == 5
        phases = {key: dict(ms=statistics.mean(x['phases'][key]['ms'] for x in xs),
                            calls=statistics.mean(x['phases'][key]['calls'] for x in xs))
                  for key in xs[0]['phases']}
        pool = {key: statistics.mean(sum(z[key] for z in x.get('pool', [])) for x in xs)
                for key in ['calls', 'park_calls', 'wait_ms', 'park_ms']}
        work = {key: statistics.mean(x['work'][key] for x in xs)
                for key, value in xs[0].get('work', {}).items() if isinstance(value, (int, float))}
        prof.append(dict(algorithm=a, graph=graph, workers=w, label=label,
                         phases=phases, pool=pool, work=work))
    result = dict(timed_observations=len(records), selected_cases=len(findings),
                  confirmed_prior_cases=sum(x['cohort'] == 'regression' for x in findings),
                  aa_failures=failures, cases=findings, profiles=prof,
                  counters=read(root / 'counter_trace_availability.json'),
                  runtime=read(root / 'runtime_code_audit.json'),
                  warning='Source-component contrasts are diagnostic. A slowdown not reproduced on this new VM has no established cause from this experiment. Profile timings overlap and are instrumented; they are not acceptance timings.')
    (out / 'diagnostic_findings.json').write_text(json.dumps(result, indent=2))
    lines = ['Uncertified-program component diagnosis', '',
             '14,080 fresh-process observations; 22 cases; 1/2 physical workers; two independent 20-pair blocks.',
             f'A/A failures after correction: {len(failures)}.',
             'Only a slowdown supported by both independent blocks and stable controls is marked reproduced.',
             'Means are milliseconds; context/API/dispatch variants are sequential source substitutions, not independent additive physical causes.', '',
             'Case | workers | block | accepted | candidate | layout only | API only | outlined | guarded | candidate change %']
    for item in findings:
        for w, row in item['widths'].items():
            for block in row['blocks']:
                v = block['variants']
                names = ['accepted', 'candidate', 'layout_only', 'api_only', 'outlined', 'guarded']
                lines.append(' | '.join([item['algorithm']+' '+item['graph'], str(w), str(block['block']+1),
                    *[f'{v[n]["whole_ms"]:.5f}' for n in names],
                    f'{(v["candidate"]["whole_ms"]/v["accepted"]["whole_ms"]-1)*100:+.3f}']))
    (out / 'diagnostic_findings.txt').write_text('\n'.join(lines)+'\n')
    doc = ['<!doctype html><meta charset="utf-8"><title>Static schedule regression diagnosis</title>',
           '<style>body{font:14px system-ui;margin:24px}table{border-collapse:collapse}td,th{padding:5px;border:1px solid #bbb;text-align:right}td:first-child{text-align:left}.scroll{overflow:auto}pre{white-space:pre-wrap}</style>',
           '<h1>Controlled component measurements</h1>',
           '<p>All times are mean whole-process milliseconds. Both independent blocks are shown. A/A and corrected component tests are in diagnostic_findings.json.</p>',
           '<div class="scroll"><table><tr>'+''.join('<th>'+html.escape(x.strip())+'</th>' for x in lines[7].split('|'))+'</tr>']
    for line in lines[8:]:
        doc.append('<tr>'+''.join('<td>'+html.escape(x.strip())+'</td>' for x in line.split('|'))+'</tr>')
    doc += ['</table></div><pre>'+html.escape('\n'.join(lines[:7]))+'</pre>',
            '<p>'+html.escape(result['warning'])+'</p>']
    (out / 'diagnostic_findings.html').write_text('\n'.join(doc))
    for x in findings:
        if x['cohort'] != 'regression':
            continue
        print(json.dumps(dict(algorithm=x['algorithm'], graph=x['graph'],
            **{str(w): {k:v for k,v in z.items() if isinstance(v, bool)} for w,z in x['widths'].items()})))
    print(f'Validated {len(records)} timing observations, {len(profiles)} separate profiles, {len(failures)} corrected A/A failures.')


if __name__ == '__main__':
    main()
