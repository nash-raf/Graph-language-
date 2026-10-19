"""Fresh final B0/accepted/Graptor comparison, including equal-double PageRank."""
import argparse,json,re,time
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--accepted');p.add_argument('--attempt',default='FinalComparison');args=p.parse_args()
accepted=args.accepted or R.read(R.OUT/'campaign_state.json')['last_accepted_version'];R.M=R.bootstrap()
with R.locked('final_comparison'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in ['B0_final',accepted]}
 protocol=dict(accepted=accepted,baseline='B0_final',fingerprints=fingerprints,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'],workers=6,repetitions=5,warmups=1,precision_control='pinned PRdouble')
 path=R.OUT/(args.attempt+'_protocol.json')
 if path.exists():assert R.read(path)==protocol
 else:R.save(path,protocol)
 result=None
 for block in [0,1]:
  records=[];phase='final_primary'
  for algorithm in R.ALGS:
   versions=[('baseline','B0_final'),('candidate',accepted),('aa','B0_final'),('graptor','graptor')]
   if algorithm=='pagerank':versions.append(('graptor_double','graptor_double'))
   start=time.time();R.state(stage=phase,algorithm=algorithm,completed=0,total=61,block_started=start,expected_completion_unix=None)
   for completed,row in enumerate(R.M.ROWS,1):
    for label,version in versions:R.invoke(args.attempt,phase+'_warmup',version,algorithm,row,0,label=label,repeat_block=block)
    for rep in range(5):
     shift=(completed+rep)%len(versions)
     for label,version in versions[shift:]+versions[:shift]:records.append(R.invoke(args.attempt,phase,version,algorithm,row,rep,label=label,repeat_block=block))
    R.progress(stage=phase,algorithm=algorithm,completed=completed,total=61,elapsed_seconds=time.time()-start,failures=sum(bool(x.get('error')) for x in records))
   R.calibrate(algorithm,phase,[61,5,[x[0] for x in versions],6],start)
  R.save(R.OUT/(args.attempt+'_block'+str(block)+'.json'),records)
  errors=[x for x in records if x.get('error')]
  aa=R.aa_control_tests(records,phase,repetitions=5) if not errors else []
  invalid=bool(errors) or any(x['regression'] for x in aa)
  result=dict(protocol=protocol,block=block,invalid=invalid,errors=errors,aa_tests=aa,cumulative=R.aggregate(records,phase),historical_graptor_over_accepted=R.aggregate(records,phase,'graptor','candidate'),equal_double_graptor_over_accepted=R.aggregate(records,phase,'graptor_double','candidate'))
  # Internal values are recovered from retained output, preserving raw records.
  internal=[]
  for item in records:
   text=R.Path(item['base']+'.out').read_text(errors='replace')
   match=re.search(r'Average\s*:\s*([0-9.eE+-]+)',text) if item['version'].startswith('graptor') else re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)',text)
   if match:internal.append(dict(item,seconds=float(match[1])))
  result['internal_historical_graptor_over_accepted']=R.aggregate(internal,phase,'graptor','candidate')
  result['internal_equal_double_graptor_over_accepted']=R.aggregate(internal,phase,'graptor_double','candidate')
  R.save(R.OUT/(args.attempt+'_block'+str(block)+'_summary.json'),result)
  if not invalid:break
 R.save(R.OUT/(args.attempt+'_summary.json'),result)
 print(json.dumps({'valid':not result['invalid'],'accepted':accepted,'cumulative':result['cumulative']['all']['overall'],'whole_process_historical':result['historical_graptor_over_accepted']['all'],'whole_process_equal_double_pagerank':result['equal_double_graptor_over_accepted']['all']['pagerank'],'internal_historical':result['internal_historical_graptor_over_accepted']['all'],'internal_equal_double_pagerank':result['internal_equal_double_graptor_over_accepted']['all']['pagerank']}),flush=True)
