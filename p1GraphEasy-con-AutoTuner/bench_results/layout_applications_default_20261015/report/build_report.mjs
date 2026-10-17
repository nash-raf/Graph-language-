import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import {fileURLToPath} from 'node:url';
import {Workbook} from '@oai/artifact-tool';

const report=path.dirname(fileURLToPath(import.meta.url));
const out=path.dirname(report),root=path.resolve(out,'../..');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const modes=['AUTO','CSR','PCSR','BCSR','SET'];
const specs=[['contact_trace','Contact exposure tracing'],['service_islands','Service outage islands'],
 ['sensor_backbone','Sensor backbone pruning'],['route_failover','Gateway failover recovery'],['trust_audit','Trust-boundary audit']];
const median=a=>{const s=[...a].sort((x,y)=>x-y);return s.length%2?s[(s.length-1)/2]:(s[s.length/2-1]+s[s.length/2])/2;};
const headers=['case','application','graph_sha256','vertices','initial_undirected_edges','event_windows','batch_edges',
 'AUTO_total_ms','CSR_total_ms','PCSR_total_ms','BCSR_total_ms','SET_total_ms','best_fixed_layout','speedup_vs_best_fixed',
 'speedup_vs_CSR','speedup_vs_PCSR','speedup_vs_BCSR','speedup_vs_SET','AUTO_min_ms','AUTO_max_ms','AUTO_cv_pct',
 'AUTO_executed_layouts','AUTO_samples','CSR_samples','PCSR_samples','BCSR_samples','SET_samples','compiler_backend','sgpl_file','autotuner_faster'];
const graphHash=hash(await fs.readFile(path.join(out,'graphs/community_projection.txt')));
const rows=[],records=[],codeHashes=new Set();
await fs.mkdir(path.join(out,'winning_codes'),{recursive:true});
for(const [kind,title] of specs){
 const name=`${kind}_events80000_batch8_default`,dir=path.join(out,'binaries',name);
 const r=JSON.parse(await fs.readFile(path.join(dir,'result.json'),'utf8'));
 const audit=JSON.parse(await fs.readFile(path.join(dir,'audit.json'),'utf8'));
 if(Object.keys(r.settings).length)throw new Error('Default backend unexpectedly has overrides');
 const source=path.join(out,'codes',`${kind}_events80000_batch8.sgpl`);
 const code=await fs.readFile(source,'utf8');
 const oldCode=await fs.readFile(path.join(root,'bench_results/layout_applications_20261015/winning_codes',`${kind}.sgpl`),'utf8');
 if(code!==oldCode)throw new Error('Program changed from the previous comparison');
 codeHashes.add(hash(code.replace(/\/\/[^\n]*/g,'').replace(/\b\d+\b/g,'#').replace(/\s+/g,'')));
 const binaries={};
 for(const mode of modes){
  if(!audit[mode].correct||!audit[mode].iterator_epochs_verified||!audit[mode].graph_fingerprints_verified)throw new Error('Audit did not pass');
  if(Math.abs(median(r.samples_ms[mode])-r.medians_ms[mode])>1e-9)throw new Error('Median mismatch');
  const ir=await fs.readFile(path.join(dir,`${mode}.ll`),'utf8');
  if(/call .*@autograph_edgemap\(/.test(ir))throw new Error('Motif backend detected');
  binaries[mode]=hash(await fs.readFile(path.join(dir,mode)));
 }
 const t=r.medians_ms,best=modes.slice(1).sort((a,b)=>t[a]-t[b])[0],won=t.AUTO<t[best];
 const saved=won?`winning_codes/${kind}.sgpl`:`codes/${kind}_events80000_batch8.sgpl`;
 if(won)await fs.writeFile(path.join(out,saved),code);
 const sample=r.samples_ms.AUTO,mean=sample.reduce((a,b)=>a+b,0)/sample.length;
 const cv=100*Math.sqrt(sample.reduce((a,b)=>a+(b-mean)**2,0)/(sample.length-1))/mean;
 const transitions=['CSR',...audit.AUTO.transitions.map(pair=>pair[1])].join(';');
 rows.push([name,title,graphHash,544,128032,80000,8,...modes.map(m=>t[m]),best,null,null,null,null,null,
  Math.min(...sample),Math.max(...sample),cv,transitions,...modes.map(m=>r.timed_runs[m]),'default',saved,won]);
 records.push({case:name,application:title,source:saved,source_sha256:hash(code),binaries_sha256:binaries,
  samples:r.timed_runs,medians_ms:t,audit,autotuner_faster:won,speedup_vs_best_fixed:t[best]/t.AUTO});
}
if(codeHashes.size!==5)throw new Error('Programs are not structurally distinct');
const wb=Workbook.create(),sheet=wb.worksheets.add('Runtime comparison');
sheet.getRange('A1:AD6').values=[headers,...rows];
sheet.getRange('N2:R2').formulas=[['=MIN(I2:L2)/H2','=I2/$H2','=J2/$H2','=K2/$H2','=L2/$H2']];
sheet.getRange('N2:R6').fillDown();
sheet.showGridLines=false;
sheet.getRange('A1:AD6').format.font={name:'Arial',size:11};
sheet.getRange('B1:B6').format.columnWidthPx=220;
sheet.getRange('H1:N6').format.columnWidthPx=135;
sheet.getRange('B1:N6').format.rowHeightPx=29;
sheet.getRange('H1:N1').format.fill='#243746';
sheet.getRange('H1:N1').format.font={name:'Arial',size:11,bold:true,color:'#FFFFFF'};
sheet.getRange('H1:N1').format.wrapText=true;
sheet.getRange('H1:N1').format.rowHeightPx=48;
sheet.getRange('H2:L6').setNumberFormat('#,##0.000');
sheet.getRange('N2:R6').setNumberFormat('0.000');
wb.recalculate();
const values=sheet.getRange('A1:AD6').values;
for(let i=0;i<5;i++)if(Math.abs(Number(values[i+1][13])-records[i].speedup_vs_best_fixed)>1e-12)throw new Error('Speedup formula mismatch');
console.log((await wb.inspect({kind:'table',range:"'Runtime comparison'!H1:N6",include:'values,formulas',tableMaxRows:6,tableMaxCols:7,maxChars:2400})).ndjson);
console.log((await wb.inspect({kind:'match',searchTerm:'#REF!|#DIV/0!|#VALUE!|#NAME\\?|#NUM!',options:{useRegex:true,maxResults:20},summary:'formula errors'})).ndjson);
const png=await wb.render({sheetName:'Runtime comparison',range:'H1:N6',scale:1,format:'png'});
await fs.writeFile(path.join(report,'runtime_preview.png'),new Uint8Array(await png.arrayBuffer()));
const encode=v=>{const s=String(v??'');return /[,"\r\n]/.test(s)?`"${s.replaceAll('"','""')}"`:s;};
const csv=values.map(row=>row.map(encode).join(',')).join('\n')+'\n';
await fs.writeFile(path.join(out,'layout_application_comparison.csv'),csv);
const imported=await Workbook.fromCSV(csv,{sheetName:'CSV verification'});
const roundtrip=imported.worksheets.getItem('CSV verification').getRange('A1:AD6').values;
if(JSON.stringify(roundtrip)!==JSON.stringify(values.map(row=>row.map(v=>String(v??'')))))throw new Error('CSV round-trip mismatch');
const experiment=JSON.parse(await fs.readFile(path.join(out,'experiment.json'),'utf8'));
const benchmarkTools={};
for(const name of ['tools/benchmark_default_layout_applications.py','tools/layout_application_benchmark.py',
 'tools/layout_application_cases.py','tools/layout_switch_benchmark.py','tools/default_layout_application_audit.c',
 'tools/layout_application_audit.c','tools/prebuild_graph_cache.cpp'])benchmarkTools[name]=hash(await fs.readFile(path.join(root,name)));
await fs.writeFile(path.join(out,'manifest.json'),JSON.stringify({...experiment,
 date:'2026-10-15',same_sources_as_motif_experiment:true,distinct_normalized_sources:codeHashes.size,
 total_measured_launches:115,timing:'Whole process wall time including loading, allocations, conversions, all native updates, analysis, profiling and output; compilation, input generation, sidecar prebuild and audits excluded.',
 audit_protocol:'Layouts checked at mutations, built-in queries, effect executors and intercepted iterator initializations. Graph fingerprints and independent semantic outputs verified. Epoch stability checked on intercepted next() calls. All native updates run in AUTO/PCSR/SET diagnostics; CSR/BCSR diagnostic wrappers omit middle recurring windows. Timed executables always run every native update.',
 benchmark_tools_sha256:benchmarkTools,cases:records},null,2)+'\n');
const notes=['# Default-backend application benchmarks','',
 'These are the exact five programs and identical input bytes from the earlier motif experiment. They now use the repaired default compiler. No GRAPH_FRONTIER_REWRITE_OFF setting is used for any variant. These are synthetic application workload models, not production trace measurements.','',
 '| Application | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Best fixed | Speedup |',
 '| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |',
 ...records.map(r=>{const t=r.medians_ms,best=modes.slice(1).sort((a,b)=>t[a]-t[b])[0];return `| ${r.application} | ${modes.map(m=>t[m].toFixed(3)).join(' | ')} | ${best} | ${r.speedup_vs_best_fixed.toFixed(3)}x |`;}),'',
 'Speedup is the fastest fixed-layout total runtime divided by AUTO total runtime. Values below 1 would indicate a slowdown. Every case is retained, regardless of outcome.','',
 '## Method','',
 '- Shared graph: 544 vertices and 128032 undirected edges, with a dense projected cohort and peripheral chain. Each program replays 80000 event windows with eight endpoint pairs. Program control flow and application outputs differ. Sources contain no manual layout commands.',
 '- AUTO and fixed variants compile identical SGPL. Only the AUTOTUNER_FORCE_LAYOUT setting changes between variants. The fixed layout is installed before the first update/traversal and retained for those operations; the loader starts in CSR and its initial conversion is included.',
 '- All thread counts are one. Executables and identical input are staged on WSL local storage. The graph-loader sidecar is prebuilt before measurement.',
 '- AUTO, PCSR and SET have seven full launches each; CSR and BCSR have one full launch each because they are much slower. Reported values are medians. Sample counts, AUTO range and variability are explicit columns in the CSV. The slow baselines do not have repeat-based variability estimates.',
 '- No full-workload warmup samples are discarded. Variant order is randomized with seed 20261015, and timing covers subprocess launch through exit. All 115 measured executions must match the independent Python reference outputs.',
 '- Separate audited executables reuse the exact measured program objects. Audits are excluded from timing. They check real layout transitions, fixed layouts, native iterator epochs, graph fingerprints, update-call counts and sampled physical mutation effects. CSR/BCSR diagnostic runs skip the middle recurring updates; all measured binaries run the entire update stream.',
 '- Actual AUTO transitions are CSR, SET, CSR in all five cases.','',
 '## Results and reproduction','',
 'The full five-variant data are in [layout_application_comparison.csv](layout_application_comparison.csv). Individual timed launches are retained in [validation_raw_runs.csv](validation_raw_runs.csv). Compiler, source, input and binary hashes and audit coverage are in [manifest.json](manifest.json).','',
 ...records.map(r=>`- [${r.application}](${r.source})`),'',
 'Run from the repository root in Ubuntu/WSL:','',
 '```sh','python3 tools/benchmark_default_layout_applications.py --resume','```','',
 'Use `codes/` or `winning_codes/` as the working directory when launching saved binaries directly, so their graph path resolves. The earlier motif results remain in `../layout_applications_20261015/`.',''];
await fs.writeFile(path.join(out,'README.md'),notes.join('\n'));
console.log(JSON.stringify(records.map(r=>({application:r.application,...r.medians_ms,speedup:r.speedup_vs_best_fixed})),null,2));
