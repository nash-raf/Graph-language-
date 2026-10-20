"""Consolidate immutable component, alignment and same-binary evidence."""
from pathlib import Path
import argparse, hashlib, html, json, statistics

def read(p):return json.loads(Path(p).read_text())

def main():
 p=argparse.ArgumentParser();p.add_argument('root');args=p.parse_args();root=Path(args.root)
 first=read(root/'diagnosis-v1/diagnostic_findings.json')
 second=read(root/'cursor-mechanism-v1/cursor_summary.json')
 addresses=read(root/'cursor-mechanism-v1/cursor_address_profiles.json')
 proof=read(root/'same-binary-proof-v1/proof_summary.json')
 protocol=read(root/'same-binary-proof-v1/proof_protocol.json')
 assert not first['aa_failures']
 assert not any(t['holm_p']<.05 for t in second['aa_tests'])
 assert not any(t['holm_p']<.05 for t in proof['aa_tests'])
 cursor_tests=[t for t in proof['tests'] if t['right']=='16']
 assert len(cursor_tests)==6 and all(t['holm_p']<.05 and t['geometric_percent']>0 for t in cursor_tests)
 assert protocol['instrumented'] and not protocol['acceptance_eligible']
 records=[]
 for a in ['bfs','sssp','cc']:
  rows=[r for r in proof['cases'] if r['algorithm']==a and r['workers']==2]
  assert len(rows)==2
  aligned=statistics.mean(r['values']['00']['whole_ms'] for r in rows)
  misaligned=statistics.mean(r['values']['16']['whole_ms'] for r in rows)
  assert all(r['values']['00']['cursor_updates_on_shared_lines']==0 for r in rows)
  records.append(dict(algorithm=a,graph=rows[0]['graph'],aligned_ms=aligned,misaligned_ms=misaligned,
    increase_percent=(misaligned/aligned-1)*100,
    cursor_updates=rows[0]['values']['00']['cursor_updates'],
    shared_updates=rows[0]['values']['16']['cursor_updates_on_shared_lines'],
    tests=[t for t in cursor_tests if t['algorithm']==a]))
 rep=[x for x in first['cases'] if x['cohort']=='regression' and x['widths']['2']['replicated_full_regression']]
 lines=['Static schedule regression causes: 2026-10-20','',
  'Established: the large Luxembourg CC regressions are caused by false sharing in the parallel preparation cursor matrix, exposed by the common context-layout change.',
  'The same defect creates controlled, statistically confirmed BFS and SSSP slowdowns. This does not establish the complete cause of every original BFS/SSSP regression.',
  '', 'Causal chain:',
  '1. The new static-schedule pointer enlarged every execution context from 568 to 576 bytes, including uncertified operations. Disabling the feature retains that layout.',
  '2. The changed allocation layout moved the cached preparation cursor buffer. On Luxembourg, the old buffer was at offset 0 modulo 64 and the candidate at offset 16, throughout all 904 parallel preparation rounds.',
  '3. With two workers the default cut has eight partitions. Each chunk has eight int64 counters: a 64-byte row. Workers own alternating chunk rows. Ordinary malloc does not ensure 64-byte row starts.',
  'The buffer is SGPL_BUF_PREP_POSITIONS. sgpl_array_count_chunks increments the chunk counter for each destination owner; sgpl_array_fill_chunks_* increments the same private cursors. The shared sgpl_execution_buffer allocator retains this workspace across rounds.',
  '4. At offset 16, row 0 spans bytes 16..79 and row 1 spans 80..143. Their distinct counters occupy the same physical cache line at bytes 64..127. Both workers repeatedly write that line during counting and filling.',
  '5. These are independent logical locations, so correctness and the effect proof remain valid. Cache coherence still transfers the shared line between cores. No extra graph/DAG work is needed to create this penalty.',
  '', 'Evidence:',
  'The component campaign changed the context header alone while retaining the original dispatcher; it reproduced the large CC penalty. Outlining or guarding the new helper did not remove it.',
  'All 183 BFS/SSSP/CC generated timed objects have identical instructions except two startup ABI-check constants. Read-only data and text relocations also match in the native audit.',
  'Luxembourg performed exactly 1,035 rounds and 28,209,639 edge occurrences. In separate phase profiles, counting changed from 95.27 to 128.00 ms and filling from 100.54 to 160.76 ms, while execution stayed near 211..215 ms. These are instrumented phase observations, not acceptance timings.',
  'Uninstrumented alignment intervention, independent block 1: candidate Luxembourg 729.191 -> 567.391 ms. Forcing a bad offset into the old build changed aligned 589.361 ms to 747.304 ms at offset 16.',
  'Independent block 2: candidate 645.290 -> 522.362 ms; old aligned 538.605 -> 659.396 ms at offset 16. All graph operations and result validations were retained.',
  'The final check uses one executable per graph with equal-length environment values and executable paths. Only cursor/record pointer offsets change; code addresses cannot change between configurations. Both independent 20-pair blocks confirm the cursor penalty after Holm correction. Identical-offset A/A controls pass.',
  'The final check directly counts writes to cursor cache lines used by multiple worker lanes: alignment removes them. Record-buffer-only offset controls do not show a corresponding confirmed slowdown. The large penalty disappears in the five-sample one-worker negative control.',
  '', 'Same-binary diagnostic means over the two blocks (instrumented; NOT acceptance measurements):']
 for x in records:lines.append(f"{x['algorithm']} {x['graph']}: aligned {x['aligned_ms']:.6f} ms, cursor offset16 {x['misaligned_ms']:.6f} ms, +{x['increase_percent']:.3f}%; {x['shared_updates']:.0f} of {x['cursor_updates']:.0f} count-pass increments on multiply-written cache lines.")
 lines += ['', 'Scope and uncertainty:',
  f"{len(rep)} of the 16 original flags passed the strict independent two-block component re-confirmation; the follow-up uses a larger contrast family and a new Kaggle VM. Other flags remain unconfirmed here, not erased from the original report.",
  'The complete allocation/alignment/performance chain is established for the large Luxembourg CC case (present under two graph names). Cursor alignment also recovers substantial time on the other targeted CC cases.',
  'On the native SSSP wing probes, both original builds used cursor offset48. Thus the controlled alignment gain is real, but a moved cursor alone cannot explain their entire original relative difference. Remaining code/data-layout interactions are unresolved. Small BFS cases and some other original flags also lack a complete individual attribution.',
  'Kaggle denied direct hardware cycle counters. Shared-line update counts are derived from actual cursor values and worker ownership; they are not measured HITM events or coherence-transaction counts.',
  'All experiments use distinct physical-core representatives (one or two workers), no SMT siblings. They ran serially per host. Raw logs, input/executable identities, failed counter attempts, correctness evidence and all controls are retained.',
  '', 'General repair:',
  'Align the preparation workspace to the hardware cache line and round each chunk row stride up to a complete cache line. Use the padded stride in counting, filling and the stable prefix pass. Preserve the raw allocation owner for release and failure handling.',
  'This changes infrastructure layout, not the execution DAG, callbacks, frontier occurrences, order, weights, thread budgets or algorithm. Keep the static-schedule optimization. Run the full 244-case correctness/timing gate before promoting a production repair.',
  'No production optimization has been reverted or promoted by this investigation. The aligned variants are diagnostic controls; no new full-dataset/Graptor victory is claimed.',
  '', 'Evidence directories:', 'diagnosis-v1: 14,080 component timings, 22 cases, 1/2 workers, two blocks, 200 separate profiles.',
  'cursor-mechanism-v1: 3,600 uninstrumented alignment timings, six cases, inverse controls, 16 address probes.',
  'same-binary-proof-v1: 675 instrumented observations, three cases, same-executable factorial controls, one-worker negative control.',
  'kaggle-v3/final_report.html: original complete 61-graph x four-algorithm timing report, preserved unchanged.']
 (root/'regression_cause_report.txt').write_text('\n'.join(lines)+'\n')
 result=dict(main_cause='False sharing in per-chunk preparation cursors; common context growth perturbs cached workspace placement.',
  complete_original_attribution=False,reconfirmed_original_cases=len(rep),same_binary_effects=records,
  original_case_status=[dict(algorithm=x['algorithm'],graph=x['graph'],**{k:v for k,v in x['widths']['2'].items() if isinstance(v,bool)}) for x in first['cases'] if x['cohort']=='regression'],
  production_changes_retained=True,production_repair_promoted=False)
 (root/'regression_cause_summary.json').write_text(json.dumps(result,indent=2))
 doc=['<!doctype html><meta charset="utf-8"><title>P1 regression cause: preparation false sharing</title>',
  '<style>body{font:15px system-ui;margin:28px;color:#172336;max-width:1500px}pre{white-space:pre-wrap;max-width:1150px}table{border-collapse:collapse}td,th{padding:7px;border:1px solid #bcc6d2;text-align:right}td:first-child{text-align:left}.line{display:grid;grid-template-columns:repeat(8,48px);border:2px solid #283e56;width:max-content;margin:5px 0}.line span{text-align:center;padding:12px 0;font-size:12px}.a{background:#acd3ff}.b{background:#ffc789}.pad{background:#e4e8ed}.shared{border-color:#bb2638}.scroll{overflow:auto}</style>',
  '<h1>A shared-runtime layout change exposed preparation false sharing</h1>',
  '<p>The large CC causal chain is established. The same defect is demonstrated in BFS and SSSP; individual attribution remains incomplete for some original flags. All optimizations remain intact.</p>',
  '<h2>Why independent counters interfere</h2><p>Each cell is 8 bytes; each outlined line is 64 bytes. Blue and orange are counters owned by different workers.</p>',
  '<p>Aligned rows:</p><div class="scroll"><div class="line">'+''.join('<span class="a">W0</span>' for _ in range(8))+'</div><div class="line">'+''.join('<span class="b">W1</span>' for _ in range(8))+'</div>',
  '<p>Rows shifted by 16 bytes:</p><div class="line">'+''.join('<span class="pad">pad</span>' for _ in range(2))+''.join('<span class="a">W0</span>' for _ in range(6))+'</div><div class="line shared">'+''.join('<span class="a">W0</span>' for _ in range(2))+''.join('<span class="b">W1</span>' for _ in range(6))+'</div></div>',
  '<p>The middle line contains distinct counters written by both workers. Logical effect independence is preserved; physical cache-line independence is lost.</p>',
  '<h2>Same-binary check</h2><p>Instrumented diagnostic means, not acceptance timings. Two independent 20-pair blocks, two physical workers, identical executable per graph.</p><table><tr><th>Algorithm / graph</th><th>Aligned ms</th><th>Cursor +16 ms</th><th>Extra time</th><th>Shared-line increments per count pass</th></tr>']
 for x in records:doc.append(f'<tr><td>{html.escape(x["algorithm"]+" "+x["graph"])}</td><td>{x["aligned_ms"]:.3f}</td><td>{x["misaligned_ms"]:.3f}</td><td>+{x["increase_percent"]:.2f}%</td><td>{x["shared_updates"]:,.0f}</td></tr>')
 doc+=['</table><h2>Original flags: follow-up status</h2><p>Strict two-block component confirmation; a failed confirmation is not proof that the original observation was impossible.</p><table><tr><th>Case</th><th>Reconfirmed</th><th>Context-only effect confirmed</th></tr>']
 for x in result['original_case_status']:doc.append('<tr><td>'+html.escape(x['algorithm']+' '+x['graph'])+'</td><td>'+str(x['replicated_full_regression'])+'</td><td>'+str(x['context_effect_confirmed'])+'</td></tr>')
 doc+=['</table><h2>Complete findings, controls and limits</h2><pre>'+html.escape('\n'.join(lines))+'</pre>',
  '<p><a href="diagnosis-v1/diagnostic_findings.html">All component cases</a> · <a href="cursor-mechanism-v1/cursor_report.html">All native alignment controls</a> · <a href="same-binary-proof-v1/proof_report.html">Same-binary controls</a> · <a href="kaggle-v3/final_report.html">Original full dataset table</a></p>']
 (root/'regression_cause_report.html').write_text('\n'.join(doc))
 print(json.dumps(result,indent=2))

if __name__=='__main__':main()
