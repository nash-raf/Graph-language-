import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import { fileURLToPath } from 'node:url';
import { Workbook } from '@oai/artifact-tool';

const here=path.dirname(fileURLToPath(import.meta.url));
const out=path.dirname(here), project=path.resolve(out,'../..');
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const modes=['AUTO','CSR','PCSR','BCSR','SET'];
const specs=[
 ['contact_trace','Contact exposure tracing','BFS exposure cohort and maximum hop count'],
 ['service_islands','Service outage islands','Connected components and largest island'],
 ['sensor_backbone','Sensor backbone pruning','490-core retention and failed-sensor eligibility'],
 ['route_failover','Gateway failover recovery','Outage BFS, alternate-gateway recovery, and DFS inspection'],
 ['trust_audit','Trust-boundary audit','DFS inventories on both sides of a revoked gateway'],
];
const median=a=>{const b=[...a].sort((a,b)=>a-b);return b.length%2?b[(b.length-1)/2]:(b[b.length/2-1]+b[b.length/2])/2;};
const graphPath=path.join(out,'graphs/community_projection.txt');
const graphHash=hash(await fs.readFile(graphPath));
const headers=['case','application','analysis','graph_file','graph_sha256','vertices','initial_undirected_edges','event_windows','batch_edges',
 'AUTO_total_ms','CSR_total_ms','PCSR_total_ms','BCSR_total_ms','SET_total_ms','best_fixed_layout','speedup_vs_best_fixed',
 'speedup_vs_CSR','speedup_vs_PCSR','speedup_vs_BCSR','speedup_vs_SET','AUTO_executed_layouts',
 'AUTO_samples','CSR_samples','PCSR_samples','BCSR_samples','SET_samples','compiler_backend','sgpl_file'];
const rows=[], records=[], normalizedHashes=new Set();
await fs.mkdir(path.join(out,'winning_codes'),{recursive:true});
for(const [kind,title,analysis] of specs){
 const caseName=`${kind}_events80000_batch8_motif`, dir=path.join(out,'binaries',caseName);
 const r=JSON.parse(await fs.readFile(path.join(dir,'result.json'),'utf8'));
 const a=JSON.parse(await fs.readFile(path.join(dir,'audit.json'),'utf8'));
 const original=path.join(out,'codes',`${kind}_events80000_batch8.sgpl`);
 const code=await fs.readFile(original,'utf8');
 if(!code.includes('edges: file "../graphs/community_projection.txt"'))throw new Error('Cases do not share the graph');
 normalizedHashes.add(hash(code.replace(/\/\/[^\n]*/g,'').replace(/\b\d+\b/g,'#').replace(/\s+/g,'')));
 for(const m of modes){
  if(!a[m].correct || !a[m].layouts_checked_on_all_calls || !a[m].graph_fingerprints_verified)throw new Error('Audit failed');
  if(Math.abs(median(r.samples_ms[m])-r.medians_ms[m])>1e-9)throw new Error('Median mismatch');
 }
 const t=r.medians_ms, best=modes.slice(1).sort((x,y)=>t[x]-t[y])[0];
 if(t[best]<=t.AUTO)throw new Error('Not a winning case');
 const layouts=['CSR',...a.AUTO.transitions.map(x=>x[1])].join(' -> ');
 const filename=`${kind}.sgpl`, counts=modes.map(m=>r.samples_ms[m].length);
 await fs.copyFile(original,path.join(out,'winning_codes',filename));
 rows.push([caseName,title,analysis,'graphs/community_projection.txt',graphHash,r.metadata.n,r.metadata.edges,r.metadata.rounds,r.metadata.batch,
  ...modes.map(m=>t[m]),best,t[best]/t.AUTO,...modes.slice(1).map(m=>t[m]/t.AUTO),layouts,...counts,'motif',`winning_codes/${filename}`]);
 const binaries={};for(const m of modes)binaries[m]=hash(await fs.readFile(path.join(dir,m)));
 records.push({case:caseName,application:title,analysis,source:`winning_codes/${filename}`,source_sha256:hash(code),
  graph_sha256:graphHash,settings:r.settings,medians_ms:t,counts,transitions:a.AUTO.transitions,binary_sha256:binaries,audits:a});
}
if(normalizedHashes.size!==5)throw new Error('Cases are parameter substitutions of the same source');
const wb=Workbook.create(),sheet=wb.worksheets.add('Application comparison');
sheet.getRange('A1:AB6').values=[headers,...rows];wb.recalculate();
console.log((await wb.inspect({kind:'table',range:"'Application comparison'!J1:P6",include:'values',tableMaxRows:6,tableMaxCols:7,maxChars:3000})).ndjson);
sheet.getRange('J1:N6').format.font={name:'Arial',size:11};sheet.getRange('J1:N1').format.font={name:'Arial',size:11,bold:true};
sheet.getRange('J1:N6').format.columnWidthPx=160;sheet.getRange('J1:N6').format.rowHeightPx=26;sheet.getRange('J2:N6').setNumberFormat('0.000');
const png=await wb.render({sheetName:'Application comparison',range:'J1:N6',scale:1,format:'png'});
await fs.writeFile(path.join(here,'runtime_preview.png'),new Uint8Array(await png.arrayBuffer()));
const matrix=sheet.getRange('A1:AB6').values;
const encode=v=>{const s=String(v??'');return /[,"\r\n]/.test(s)?`"${s.replaceAll('"','""')}"`:s;};
const csv=matrix.map(r=>r.map(encode).join(',')).join('\n')+'\n';
await fs.writeFile(path.join(out,'layout_application_wins.csv'),csv);
const imported=await Workbook.fromCSV(csv,{sheetName:'CSV verification'});
if(JSON.stringify(imported.worksheets.getItem('CSV verification').getRange('A1:AB6').values)!==JSON.stringify(matrix.map(r=>r.map(v=>String(v??'')))))throw new Error('CSV round-trip mismatch');
const sourceHashes={};
for(const p of ['GraphProgram','AutoTunerPass.cpp','autotuner_runtime.c','graph_mutation_runtime.c','bfs_runtime_src.cpp','dfs_runtime_src.cpp',
 'tools/layout_application_cases.py','tools/layout_application_benchmark.py','tools/layout_application_audit.c','tools/audit_layout_applications.py'])sourceHashes[p]=hash(await fs.readFile(path.join(project,p)));
await fs.writeFile(path.join(out,'manifest.json'),JSON.stringify({date:'2026-10-15',timezone:'Asia/Dhaka',
 same_graph_for_all_cases:true,graph_sha256:graphHash,distinct_normalized_sources:normalizedHashes.size,
 backend:'Supported motif lowering, GRAPH_FRONTIER_REWRITE_OFF=1 for all 25 variants',
 timing:'Whole subprocess wall time, including graph loading, cache initialization, layout conversions, updates, analysis and output. Compilation, fixture generation, sidecar prebuild and diagnostic audits are excluded.',
 input_cache:'Same input bytes, prebuilt loader sidecar, WSL local staging filesystem',
 sampling:'Contact tracing: 3 launches of every variant. Other cases: 7 AUTO/PCSR/SET launches and 1 full CSR/BCSR launch. Variant order randomized with seed 20261015.',
 audit_sampling:'Every graph-call layout is checked. Mutation effects are probed in the first and last two windows and all permanent changes. AUTO/PCSR/SET audits run all native updates. CSR/BCSR diagnostic wrappers omit native updates in middle repeated windows; timed executables always perform the full workload.',
 sourceHashes,cases:records},null,2)+'\n');
const notes=[ '# Distinct SGPL application workloads', '',
 'Five different programs use the same 544-vertex, 128032-edge input graph. These are synthetic workload models, not measurements from production traces.', '',
 '| Workflow | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Fastest fixed | Speedup |',
 '| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |',
 ...rows.map(r=>`| ${r[1]} | ${r.slice(9,14).map(x=>x.toFixed(3)).join(' | ')} | ${r[14]} | ${r[15].toFixed(3)}x |`), '',
 'Times are total program wall times. Speedup is the fastest fixed-layout time divided by AUTO time.',
 'The full comparison is [layout_application_wins.csv](layout_application_wins.csv). Raw launches are retained in [validation_raw_runs.csv](validation_raw_runs.csv).', '',
 '## Winning programs and useful results', '',
 ...records.map(r=>`- [${r.source.split('/').pop()}](${r.source}): ${r.analysis}.`), '',
 'Contact tracing expires old contacts, introduces a new window, then reports potential exposure cohorts and furthest hop count.',
 'Outage detection identifies two disconnected service islands and their largest membership count.',
 'Backbone pruning maintains a supplied degree cache with every radio-link change, then computes the 490-core. It retains 511 sensors and excludes the failed sensor.',
 'Failover recovery reports stranded clients, commissions an alternate gateway, and verifies that all 544 vertices are reachable again.',
 'Trust auditing reports the 512-vertex authorized component and the isolated 32-vertex component after a gateway revocation.', '',
 'The shared fixture contains a dense projected cohort and a sparse peripheral chain. All programs replay recurring hot-edge event windows, but their event semantics, analysis code, and outputs differ. The benchmark source is generated with literal endpoint IDs because this SGPL grammar accepts only literal mutation endpoints.', '',
 '## Compiler and measurement protocol', '',
 '- All cases use the supported motif backend: `GRAPH_FRONTIER_REWRITE_OFF=1` during compilation, for AUTO and every fixed variant. Ordinary iterator loops in the current default backend pin the graph to CSR, so this report does not claim these gains for default lowering.',
 '- AUTO has no layout-forcing variable. Its executed schedule is CSR, then SET for the update burst, then CSR for analysis. The SGPL sources contain no manually inserted layout operations.',
 '- Fixed variants compile identical source with `AUTOTUNER_FORCE_LAYOUT=CSR`, `PCSR`, `BCSR`, or `SET`. The loader starts in CSR; install the fixed representation before the first traversal or update and keep it through the remaining traversal and mutation phases. Include the initial conversion in runtime.',
 '- SGPL, OpenMP, and Roaring thread counts are one. Input and executables are staged on WSL local storage, with the graph-loader sidecar prepared before timing.',
 '- Timers cover subprocess launch through exit, including loading, allocation, conversions, all updates, analysis, profiling, and printed results. No kernel-only timings are used.',
 '- Contact tracing has three full launches per variant. The remaining four cases have seven launches each of AUTO, PCSR, and SET, and one full launch each of CSR and BCSR. Sample counts are explicit CSV columns. Those slow baselines are orders of magnitude away from the winning competitor.',
 '- All 107 full timed launches were checked against independent Python reference traversals, components, and k-core calculations. Traversal outputs are compared as vertex inventories, allowing different valid traversal orders.',
 '- Separate diagnostics check every graph-call layout, actual AUTO transitions, graph fingerprints, exact update-call counts, and physical edge changes in the first and last two event windows plus permanent changes. AUTO/PCSR/SET diagnostic runs execute all native updates. CSR/BCSR diagnostics skip native work in middle recurring windows; this optimization is absent from timed executables.', '',
 '## Reproduce', '', 'Run from the repository root under Ubuntu/Linux:', '', '```bash',
 'python3 tools/layout_application_benchmark.py --rounds 80000 --modes AUTO,CSR,PCSR,BCSR,SET --adaptive-confirmation --warmups 0 --timeout 600',
 'python3 tools/audit_layout_applications.py', '```', '',
 'The first command uses seven fast-variant runs and one slow-variant run for every case. The retained contact-tracing measurements instead use three of each, as recorded in the CSV.',
 'Use `codes/` or `winning_codes/` as the working directory when executing a saved binary directly, so the shared graph path resolves.',
 'Source, compiler and binary hashes, exact sample counts, and audit coverage are in [manifest.json](manifest.json). Generated IR and logs are in `binaries/<case>/`.', '',
 'Cases were selected in a search for layout-switch wins. The earlier graph-size variants are retained separately in `../layout_switch_20261015/` and do not count as these five application cases.', '' ];
await fs.writeFile(path.join(out,'README.md'),notes.join('\n'));
console.log(JSON.stringify(rows.map(r=>({application:r[1],best_fixed:r[14],speedup:r[15]})),null,2));
