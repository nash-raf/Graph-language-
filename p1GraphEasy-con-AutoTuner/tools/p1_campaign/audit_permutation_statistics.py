"""Require exact p-value agreement with frozen scalar campaign reports."""
from pathlib import Path
import argparse, hashlib, json, time
import numpy as np
from portable_campaign import permutation_pvalue
def main():
    p=argparse.ArgumentParser();p.add_argument('results');p.add_argument('output');a=p.parse_args()
    root=Path(a.results);report=json.loads((root/'comparison.json').read_text());manifest=json.loads((root/'manifest.json').read_text())
    checked=[];start=time.monotonic()
    for phase,summary in report['summary'].items():
        rows=json.loads((root/(phase+'.json')).read_text());groups={}
        for r in rows:groups.setdefault((r['algorithm'],r['graph'],r['label']),[]).append(r)
        vectors={key:np.array([r['seconds'] for r in sorted(xs,key=lambda r:r['repeat'])]) for key,xs in groups.items()}
        for two,field,label in [(False,'tests','candidate'),(True,'aa_tests','AA')]:
            by_alg={alg:[] for alg in ['bfs','sssp','cc','pagerank']}
            for alg in by_alg:
                for row in manifest['cases']:
                    key=(alg,row['graph']);d=np.log(vectors[(*key,label)]/vectors[(*key,'accepted')]);by_alg[alg].append(d)
                    expected=next(t['p'] for t in summary[field] if t['kind']=='case' and t['algorithm']==alg and t['graph']==row['graph'])
                    actual=permutation_pvalue(d,two)
                    assert actual==expected,(phase,field,key,actual,expected)
                    checked.append({'phase':phase,'field':field,'algorithm':alg,'graph':row['graph'],'p':actual})
            for alg,ds in by_alg.items():
                actual=permutation_pvalue(np.mean(ds,axis=0),two)
                expected=next(t['p'] for t in summary[field] if t['kind']=='algorithm' and t['algorithm']==alg)
                assert actual==expected,(phase,field,alg,actual,expected)
                checked.append({'phase':phase,'field':field,'algorithm':alg,'p':actual})
    result={'exact_p_values':len(checked),'passed':True,'seconds':time.monotonic()-start,
            'reference_sha256':hashlib.sha256((root/'comparison.json').read_bytes()).hexdigest(),'seed':2358,'resamples':100000,'checks':checked}
    Path(a.output).write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items() if k!='checks'}))
if __name__=='__main__':main()
