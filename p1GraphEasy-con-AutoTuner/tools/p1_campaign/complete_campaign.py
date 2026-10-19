"""Seal a completed campaign only after validating its accepted source and reports."""
import datetime
import runner as R
R.M=R.bootstrap()
with R.locked('complete_campaign'):
 state=R.read(R.OUT/'campaign_state.json');accepted=state['last_accepted_version']
 assert R.git('branch','--show-current')==state['accepted_branch']
 assert not R.git('status','--porcelain'),'Commit the delivered branch first'
 R.verify_inputs();fingerprint=R.verify_fingerprint(accepted);_,_,identity=R.freeze(accepted)
 final=R.read(R.OUT/'FinalComparison_summary.json');assert final and not final['invalid'] and final['protocol']['accepted']==accepted
 for v in ['B0_final','R2a3',accepted]:
  values=R.read(R.OUT/(v+'_validation_workers.json'));assert values and len(values)==732 and all(x.get('validation') and not x.get('error') for x in values),v
 outcomes={}
 for n in range(1,5):
  versions={p.name.removesuffix('_decision.json'):R.read(p) for p in R.OUT.glob('R'+str(n)+'*_decision.json')}
  assert versions,'Missing recommendation '+str(n)
  for v,d in versions.items():
   if d['decision']=='reject':assert (R.OUT/(v+'_rejection.txt')).exists(),'Missing rejection investigation '+v
  outcomes[str(n)]={v:d['decision'] for v,d in sorted(versions.items())}
 report=R.OUT/'final_report.txt';assert report.exists()
 checkpoint=dict(accepted_version=accepted,accepted_source=identity,accepted_executable_fingerprint=fingerprint,delivery_commit=R.git('rev-parse','HEAD'),branch=state['accepted_branch'],outcomes=outcomes,final_report_sha256=R.sha(report),final_comparison_sha256=R.sha(R.OUT/'FinalComparison_summary.json'),build_protocol_sha256=R.sha(R.OUT/'build_protocol.json'),profile_work_audit_sha256=R.sha(R.OUT/'profile_work_audit.json'))
 path=R.OUT/'final_checkpoint.json'
 if path.exists():assert R.read(path)==checkpoint
 else:R.save(path,checkpoint)
 state.update(status='complete',candidate_branch=None,current_attempt=None,delivery_commit=checkpoint['delivery_commit'],completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),final_checkpoint=str(path))
 R.save(R.OUT/'campaign_state.json',state);R.progress(stage='campaign_complete',version=accepted,completed=244,total=244,failures=0);print(path)
