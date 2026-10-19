"""Audit retained primary and confirmation gates without changing observations.

Preserve pre-audit decisions/summaries and publish the explicit control-family
audit. This also covers old runs made before bidirectional A/A checking existed.
"""
import argparse,json
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('versions',nargs='+');args=p.parse_args();R.M=R.bootstrap()
with R.locked('audit_gates'):
 for version in args.versions:
  for phase,repetitions in [('primary',5),('confirmation',20)]:
   sp=R.OUT/(version+'_'+phase+'_summary.json')
   if not sp.exists():continue
   summary=R.read(sp)
   if summary.get('failures'):continue
   rp=R.OUT/(version+'_'+phase+('_repeat' if summary.get('block') else '')+'.json');records=R.read(rp)
   if len([x for x in records if x['label']=='aa'])!=244*repetitions:continue
   audit=R.OUT/(version+'_'+phase+'_gate_audit.json')
   if audit.exists():continue
   before=sp.with_name(sp.stem+'_before_gate_audit.json')
   if not before.exists():R.save(before,summary)
   R.state(stage='gate_audit',version=version,algorithm=phase,completed=0,total=744,expected_completion_unix=None)
   tests=summary.get('tests') or R.regression_tests(records,phase,repetitions=repetitions)
   aa=R.aa_control_tests(records,phase,repetitions,summary.get('aa_tests'))
   summary.update(tests=tests,confirmed_regressions=[x for x in tests if x['regression']],aa_tests=aa,aa_invalid=any(x['regression'] for x in aa))
   R.save(audit,dict(phase=phase,version=version,records_sha256=R.sha(rp),candidate_family=248,aa_family=496,aa_invalid=summary['aa_invalid'],confirmed_regressions=summary['confirmed_regressions'],aa_bias=[x for x in aa if x['regression']]))
   R.save(sp,summary)
  dp=R.OUT/(version+'_decision.json');decision=R.read(dp)
  if not decision:continue
  before=dp.with_name(dp.stem+'_before_gate_audit.json')
  if not before.exists():R.save(before,decision)
  primary=R.read(R.OUT/(version+'_primary_summary.json'),{});confirmation=R.read(R.OUT/(version+'_confirmation_summary.json'),{})
  invalid=primary.get('aa_invalid') or confirmation.get('aa_invalid')
  regression=primary.get('confirmed_regressions',[])+confirmation.get('confirmed_regressions',[])
  if invalid:decision.update(decision='invalid_aa',promotable=False,reason='bidirectional_aa_bias')
  elif regression:decision.update(decision='reject',promotable=False,reason='confirmed_case_or_algorithm_regression')
  decision.update(primary_confirmed_regressions=primary.get('confirmed_regressions',[]),gate_audit_completed=True)
  R.save(dp,decision)
  print(json.dumps({'version':version,'decision':decision['decision'],'primary_aa_invalid':primary.get('aa_invalid'),'confirmation_aa_invalid':confirmation.get('aa_invalid'),'regressions':regression}),flush=True)
