"""Audit a completed local comparison and make failed stability gates explicit."""
from pathlib import Path
import argparse, hashlib, html, json, math, os, statistics

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--output',required=True)
args=p.parse_args();os.environ['P1_CAMPAIGN_OUTPUT']=args.output
import runner as R

def main():
    R.M=R.bootstrap()
    with R.locked('audit_local_current_comparison'):
        protocol=R.read(R.OUT/'protocol.json');R.verify_inputs()
        assert R.sha(Path(__file__).with_name('local_current_comparison.py'))==protocol['harness_sha256']
        R.freeze(protocol['current'])
        for v,fp in protocol['fingerprints'].items():assert R.verify_fingerprint(v)==fp
        blocks={}
        for phase,n in [('primary',5),('confirmation',20),('confirmation_retry1',20)]:
            result=R.read(R.OUT/(phase+'_comparison.json'))
            if result is None:continue
            rows=result['cases'];s=result['summary'];raw=R.read(R.OUT/(phase+'.json'))
            assert len(rows)==244 and len(raw)==1281*n and s['observations']==len(raw)
            assert len({(x['algorithm'],x['graph']) for x in rows})==244
            assert all(sum(x['algorithm']==a for x in rows)==61 for a in R.ALGS)
            assert all(x['workers']==6 and not x.get('error') for x in raw)
            for label in ['accepted','candidate','graptor']:
                assert math.isclose(s['totals_ms'][label],sum(x['mean_ms'][label] for x in rows),rel_tol=1e-13)
            blocks[phase]={'observations':len(raw),'valid_for_inference':s['valid_for_inference'],
                'aa_failures':s['aa_failures'],'totals_ms':s['totals_ms'],
                'wins':sum(x['p1_wins'] for x in s['by_algorithm'].values())}
        final=R.read(R.OUT/'final_summary.json');assert final['phase'] in blocks
        assert R.read(R.OUT/'validated.json')['passed']
        assert len(R.read(R.OUT/'exact_pagerank_vectors.json'))==61
        warning=''
        if not final['valid_for_inference']:
            warning=('STABILITY GATE FAILED: Both confirmation blocks failed an identical-binary control. '
                'The protocol allows one repeat, which is now exhausted. Times, win counts and ratios are '
                'descriptive measurements, not certified improvements. No promotion or clean no-regression '
                'claim is supported. The zero confirmed-regression count is suppressed by the failed gate; '
                'it does not establish absence of regressions. All code and observations are retained.\n')
            for phase in ['confirmation','confirmation_retry1']:
                for x in blocks.get(phase,{}).get('aa_failures',[]):
                    warning+=f"{phase}: {x['family']} / {x['algorithm']} / {x.get('graph','algorithm aggregate')}; log-paired change {math.expm1(x['log_ratio'])*100:+.3f}%; Holm p={x['holm_p']:.8g}.\n"
            txt=R.OUT/'final_report.txt';original=txt.read_text()
            if not original.startswith('STABILITY GATE FAILED:'):txt.write_text(warning+'\n'+original)
            page=R.OUT/'final_report.html';original=page.read_text()
            if 'id="stability-warning"' not in original:
                page.write_text(original.replace('<h1>',
                    '<aside id="stability-warning" style="background:#fff1d6;border:2px solid #9b4d00;padding:16px"><pre>'+html.escape(warning)+'</pre></aside><h1>',1))
        audit={'passed_integrity_checks':True,'statistical_gate_passed':final['valid_for_inference'],
            'source_identity':protocol['source_identity'],'fingerprints':protocol['fingerprints'],
            'blocks':blocks,'total_timed_observations':sum(x['observations'] for x in blocks.values()),
            'raw_records':str(R.ROOT/'runs'/'LocalCurrentPC20261020'),
            'warning':warning,'native_changes_made_during_measurement':False}
        R.save(R.OUT/'closeout_audit.json',audit)
        hashes={x.name:R.sha(x) for x in R.OUT.iterdir() if x.is_file() and x.suffix in ['.json','.html','.txt'] and x.name not in ['SHA256SUMS.json','status.json']}
        R.save(R.OUT/'SHA256SUMS.json',hashes)
        print(json.dumps(audit),flush=True)

if __name__=='__main__':main()
