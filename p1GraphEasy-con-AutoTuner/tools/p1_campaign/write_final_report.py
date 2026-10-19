"""Write the completed campaign from immutable decisions and fresh comparison."""
import datetime,json
import runner as R
state=R.read(R.OUT/'campaign_state.json');final=R.read(R.OUT/'FinalComparison_summary.json')
assert final and not final['invalid'],'A valid complete final comparison is required'
assert R.read(R.OUT/'profile_work_audit.json'),'Audit the complete final full-domain work profile first'
accepted=state['last_accepted_version'];assert final['protocol']['accepted']==accepted
decisions={p.stem.removesuffix('_decision'):R.read(p) for p in R.OUT.glob('*_decision.json')}
assert any(v.startswith('R4') for v in decisions),'Recommendation 4 has not reached its gate'
lines=['Sequential P1 optimization campaign — measured on six physical cores',
 'Generated '+datetime.datetime.now(datetime.timezone.utc).isoformat(),
 'Accepted branch: '+state['accepted_branch'], 'Workspace: '+state['worktree'],
 'Last accepted executable set: '+accepted,'Accepted checkpoint: '+state['accepted_commit'],
 'Delivered branch HEAD: '+R.git('rev-parse','HEAD'),
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
lines+=['','Largest measured internal gaps (reported regions differ; diagnostic priorities):']
for algorithm in R.ALGS:
 label='graptor_double' if algorithm=='pagerank' else 'graptor'
 cases=[x for x in final['per_case'] if x['algorithm']==algorithm and x['internal_ms'].get(label) and x['internal_ms'].get('candidate')]
 for x in sorted(cases,key=lambda x:x['internal_ms']['candidate']/x['internal_ms'][label],reverse=True)[:3]:
  p=x['internal_ms']['candidate'];g=x['internal_ms'][label]
  lines.append(f"  {algorithm} {x['graph']}: P1={p:.3f} ms, Graptor={g:.3f} ms, P1/Graptor={p/g:.3f} ({'equal-double' if algorithm=='pagerank' else 'pinned historical'})")
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
 'promotion requirements. A later primary-gate audit also found a PageRank',
 'algorithm loss for R1a1; its residual mechanism remains unresolved. R2a2',
 'showed a confirmed PageRank loss whose precise',
 'cause remains unresolved after disable, section-layout and cross-link controls.',
 'The final R2 variant independently passed the full gate; that does not explain',
 'the earlier loss. See the original investigation and gate audit supplements.',
 '', 'R3 reduced record copying but added topology construction, descriptor work',
 'and native-ordinal gathers. Parallel preparation and narrower descriptors',
 'each recovered measured cost in component controls, but did not recover',
 'accepted performance. In the final Wing SSSP diagnostic, descriptor payload',
 'was already below accepted record bytes while traversal/join remained about',
 'three times as expensive. Individual load/branch/cache contributions inside',
 'that path were not separately established. See all three R3 investigations.',
 '', 'R4a1 isolated cold incoming-index construction as the sparse PageRank',
 'failure: index-only same-binary controls reproduced added cost, and disabling',
 'the specialization recovered it. Parallel construction was tested in R4a2.',
 'Later revision outcomes and any unresolved inactive-algorithm changes must',
 'be read from their rejection investigations; aggregate PageRank gains do not',
 'override any confirmed case or algorithm slowdown.',
 '', 'R4a2 did not repeat its original CC aggregate loss in the separate 61-case',
 '20-pair retest, but WormNet CC then showed a reproducible loss. A shim-free',
 'control preserved the old executor-section order with empty ELF declarations;',
 'all unrelocated object code sections remained byte-identical, while WormNet',
 'recovered from 8.928 to 8.339 ms (accepted baseline 8.444 ms). This isolates',
 'a code-placement intervention, not a specific cache or branch-predictor cause.',
 'An earlier nominal data-layout control also linked an ABI shim and is',
 'explicitly treated as confounded in the corrected R4a2 investigation.',
 '', 'Next work should follow the measured bottlenecks. Keep the accepted lean',
 'records and fixed 50-us waiting policy. For direct traversal, make the gather',
 'executor competitive before adding more descriptor machinery; a smaller',
 'payload alone is insufficient. Inlining certified pair bodies within the',
 'accepted lean-record traversal is a useful next experiment to isolate callback',
 'dispatch cost while keeping its preparation, append accounting and ordering.',
 'For full-graph lowering, retain ordered double',
 'accumulation and account for cold index construction. Derive any future',
 'selection rule from measured work and amortization costs, never graph names.',
 'Parallel source maps over independent sources are another measurable target,',
 'subject to the same stable-read/effect proofs and exact floating-point checks.',
 'Keep inactive compiler/runtime paths isolated and require component controls',
 'to reproduce and recover any generic-path loss before another campaign.',
 'Use the internal execution comparison to prioritize kernel work; the primary',
 'whole-process comparison includes loading and cannot establish a kernel win.',
 '', 'The final accepted executable set and repaired B0 were independently checked',
 'on all 244 cases at each of 1, 2 and 6 physical workers (732 results each).',
 '', 'The original production checkout and previous investigation were preserved.',
 'Automatic approval review rejected removal of the incomplete D:/p1camp',
 'checkout; it remains in place. The valid campaign worktree is on C:.']
path=R.OUT/'final_report.txt';assert not path.exists(),'Do not overwrite an issued final report';path.write_text('\n'.join(lines)+'\n');print(path)
