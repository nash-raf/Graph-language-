"""Write the completed campaign from immutable decisions and fresh comparison."""
import datetime,json
import runner as R
state=R.read(R.OUT/'campaign_state.json');final=R.read(R.OUT/'FinalComparison_summary.json')
assert final and not final['invalid'],'A valid complete final comparison is required'
accepted=state['last_accepted_version'];assert final['protocol']['accepted']==accepted
decisions={p.stem.removesuffix('_decision'):R.read(p) for p in R.OUT.glob('*_decision.json')}
assert any(v.startswith('R4') for v in decisions),'Recommendation 4 has not reached its gate'
lines=['Sequential P1 optimization campaign — measured on six physical cores',
 'Generated '+datetime.datetime.now(datetime.timezone.utc).isoformat(),
 'Accepted branch: '+state['accepted_branch'], 'Workspace: '+state['worktree'],
 'Last accepted executable set: '+accepted,'Accepted checkpoint: '+state['accepted_commit'],
 'Raw evidence: C:/p1campaign-results and /home/kazi/p1_campaign_20261019','',
 'B0_final is the rebuilt baseline after the 64-byte Graph allocation and zero-',
 'initialization repair. That repair is a correctness prerequisite, not a gain.',
 'All measurements ran locally with CPUs 0,2,4,6,8,10, one logical CPU from each',
 'of six physical cores. SMT siblings were excluded. No 32-worker result is',
 'claimed. Inputs, directionality, sources, weights, program sources, settings',
 'and toolchains are pinned in manifest.json and build_protocol.json.','',
 'Recommendation outcomes:']
for n in range(1,5):
 passing=[v for v,d in decisions.items() if v.startswith('R'+str(n)) and d.get('promotable') and d['decision']=='accept']
 lines.append('  Recommendation '+str(n)+': '+('accepted '+', '.join(sorted(passing)) if passing else 'rejected; no candidate entered the cumulative chain'))
lines+=['','Attempt decisions (speedup means baseline/candidate; >1 favors candidate):']
for version,d in sorted(decisions.items()):
 g=d.get('aggregate',{}).get('all',{}).get('overall',{});primary=R.read(R.OUT/(version+'_primary_summary.json'))
 primary_g=primary.get('aggregate',{}).get('all',{}).get('overall',{}) if primary else {}
 def number(x):return 'not measured' if x is None else f'{x:.6f}'
 lines.append(f"{version}: {d['decision']}, baseline={d.get('baseline')}, primary mean/median={number(primary_g.get('speedup_mean'))}/{number(primary_g.get('speedup_median'))}, final gate mean/median={number(g.get('speedup_mean'))}/{number(g.get('speedup_median'))}")
 for test in d.get('confirmed_regressions',[]):lines.append('  Confirmed loss: '+test['algorithm']+' '+test.get('graph','algorithm aggregate')+' ratio='+number(test['ratio_candidate_over_baseline'])+' Holm p='+str(test['p_holm']))
 if d.get('reason'):lines.append('  '+str(d['reason']))
lines+=['','Promotion requires >1% aggregate mean and median geometric speedup in',
 'primary and independent 20-pair confirmation across all 244 cases, no Holm-',
 'confirmed case/algorithm loss, and passing identical-binary A/A controls.',
 'Permutation tests use seed 2358, 100,000 resamples and a 248-test family at 5%.',
 'A/A controls test both directions with combined multiplicity correction.',
 'Primary uses one warmup and five fresh processes per case/variant. Profiling',
 'and component controls never enter these acceptance results.','',
 'Fresh final comparison (including all loading, setup and index construction):']
for cohort in ['all','main50','deduplicated']:
 lines.append('\n'+cohort+' cohort:')
 for algorithm in R.ALGS+['overall']:
  g=final['cumulative'][cohort][algorithm];hist=final['historical_graptor_over_accepted'][cohort][algorithm]
  lines.append(f"  {algorithm}: {g['cases']} cases; cumulative B0/accepted mean={g['speedup_mean']:.6f}, median={g['speedup_median']:.6f}; historical Graptor/accepted mean={hist['speedup_mean']:.6f}, median={hist['speedup_median']:.6f}")
 pr=final['equal_double_graptor_over_accepted'][cohort]['pagerank']
 lines.append(f"  equal-double PageRank Graptor/accepted: mean={pr['speedup_mean']:.6f}, median={pr['speedup_median']:.6f}")
lines+=['','Reported internal execution regions (Graptor/accepted; >1 favors P1):']
for algorithm in R.ALGS:
 g=final['internal_historical_graptor_over_accepted']['all'][algorithm]
 lines.append(f"  {algorithm}: historical mean={g['speedup_mean']:.6f}, median={g['speedup_median']:.6f}")
pr=final['internal_equal_double_graptor_over_accepted']['all']['pagerank'];lines.append(f"  equal-double PageRank: mean={pr['speedup_mean']:.6f}, median={pr['speedup_median']:.6f}")
lines+=['','Whole-process timing is the primary metric. P1 and Graptor have different',
 'loading/preprocessing paths and reported execution-region boundaries. A',
 'whole-process lead must not be presented as proof of a kernel lead. The',
 'historical and equal-double PageRank controls are both reported above.','',
 'Rejected native patches and provenance are in patches/. Trial branches and',
 'frozen source/executable snapshots retain rejected code outside the accepted',
 'chain. Per-rejection causal conclusions and unresolved questions are in:']
lines += ['  '+p.name for p in sorted(R.OUT.glob('*_rejection.txt'))]
lines+=['','R1 shared-wake regressions were isolated to excluded helpers waking and',
 'parking; selective wake recovered that mechanism but did not meet aggregate',
 'promotion requirements. R2a2 showed a confirmed PageRank loss whose precise',
 'cause remains unresolved after disable, section-layout and cross-link controls.',
 'The final R2 variant independently passed the full gate; that does not explain',
 'the earlier loss. See the original investigation and gate audit supplements.',
 '', 'The original production checkout and previous investigation were preserved.',
 'Automatic approval review rejected removal of the incomplete D:/p1camp',
 'checkout; it remains in place. The valid campaign worktree is on C:.']
path=R.OUT/'final_report.txt';assert not path.exists(),'Do not overwrite an issued final report';path.write_text('\n'.join(lines)+'\n');print(path)
