"""Promote only a completed passing gate; squash trial history into accepted chain."""
import argparse
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');args=p.parse_args()
with R.locked('promote'):
 decision=R.read(R.OUT/(args.version+'_decision.json'));assert decision and decision.get('promotable') and decision['decision']=='accept'
 primary=R.read(R.OUT/(args.version+'_primary_summary.json'))
 assert not any(x.get('regression') for x in primary['aa_tests']+decision['aa_tests'])
 assert not primary.get('confirmed_regressions') and not decision.get('confirmed_regressions')
 assert not R.git('status','--porcelain'),'Commit the trial implementation and reusable tooling first'
 identity=R.read(R.ROOT/'versions'/args.version/'identity.json');R.verify_inputs();R.verify_fingerprint(args.version);R.freeze(args.version)
 state=R.read(R.OUT/'campaign_state.json');branch=R.git('branch','--show-current')
 assert branch!=state['accepted_branch'],'Run promotion from its trial branch'
 R.git('switch',state['accepted_branch']);R.git('merge','--squash',branch)
 R.git('commit','-m','Accept '+args.version+' after complete correctness and paired benchmark gates')
 accepted=R.git('rev-parse','HEAD');R.freeze(args.version)
 tag='codex/p1-accepted-'+args.version.lower();R.git('tag',tag)
 checkpoint=dict(version=args.version,source_commit=identity['commit'],source_hash=identity['source_hash'],accepted_commit=accepted,accepted_tag=tag,
  baseline=decision['baseline'],primary=primary['aggregate'],confirmation=decision['aggregate'],aa_passed=True,confirmed_regressions=[])
 path=R.OUT/'checkpoints'/(args.version+'.json');assert not path.exists();R.save(path,checkpoint)
 state.setdefault('completed_attempts',{})[args.version]='accepted after complete primary and confirmation gates'
 state.update(last_accepted_version=args.version,accepted_commit=accepted,candidate_branch=None,current_attempt=None);R.save(R.OUT/'campaign_state.json',state)
 print(str(path))
