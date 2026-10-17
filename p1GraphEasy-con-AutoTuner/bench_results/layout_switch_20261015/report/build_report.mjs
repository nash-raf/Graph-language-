import fs from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { Workbook } from '@oai/artifact-tool';

const here=path.dirname(fileURLToPath(import.meta.url));
const root=path.dirname(here);
const selected=JSON.parse(await fs.readFile(path.join(root,'selected_cases.json'),'utf8'));
const headers=['case','vertices','undirected_edges','batch_edges','update_cycles','traversals_before','traversals_after',
  'AUTO_total_ms','CSR_total_ms','PCSR_total_ms','BCSR_total_ms','SET_total_ms',
  'speedup_vs_CSR','speedup_vs_PCSR','speedup_vs_BCSR','speedup_vs_SET',
  'best_fixed_layout','speedup_vs_best_fixed','AUTO_executed_layouts','timed_runs_per_variant','sgpl_file'];
const median=a=>{const s=[...a].sort((a,b)=>a-b);return s.length%2?s[(s.length-1)/2]:(s[s.length/2-1]+s[s.length/2])/2;};
const rows=[];
for(const name of selected){
  const dir=path.join(root,'binaries',name);
  const r=JSON.parse(await fs.readFile(path.join(dir,'result.json'),'utf8'));
  const a=JSON.parse(await fs.readFile(path.join(dir,'audit.json'),'utf8'));
  const c=r.config;
  for(const m of ['AUTO','CSR','PCSR','BCSR','SET']){
    if(!a[m].correct)throw new Error(`${name}/${m}: audit failed`);
    if(!a[m].mutation_effects_checked)throw new Error(`${name}/${m}: mutation effects were not verified`);
    if(Math.abs(median(r.samples_ms[m])-r.medians_ms[m])>1e-9)throw new Error('Median mismatch');
    if(r.samples_ms[m].length!==7)throw new Error('Expected seven final measurements');
  }
  const times=r.medians_ms;
  const fixed=['CSR','PCSR','BCSR','SET'].sort((x,y)=>times[x]-times[y]);
  const speedup=times[fixed[0]]/times.AUTO;
  if(speedup<=1)throw new Error(`${name} did not beat best fixed layout`);
  const layouts=['CSR',...a.AUTO.transitions.map(t=>t[1])].join(' -> ');
  rows.push([name,c.n,c.n*c.degree,c.inserts,c.churn,c.scans,c.tail_scans??c.scans,
    times.AUTO,times.CSR,times.PCSR,times.BCSR,times.SET,
    times.CSR/times.AUTO,times.PCSR/times.AUTO,times.BCSR/times.AUTO,times.SET/times.AUTO,
    fixed[0],speedup,layouts,7,`winning_codes/${name}.sgpl`]);
  await fs.mkdir(path.join(root,'winning_codes'),{recursive:true});
  await fs.copyFile(path.join(root,'codes',`${name}.sgpl`),path.join(root,'winning_codes',`${name}.sgpl`));
}
if(rows.length!==5)throw new Error('Expected five winning cases');
const workbook=Workbook.create();
const sheet=workbook.worksheets.add('Runtime comparison');
sheet.getRange('A1:U6').values=[headers,...rows];
workbook.recalculate();
console.log((await workbook.inspect({kind:'table',range:"'Runtime comparison'!H1:R6",include:'values',tableMaxRows:6,tableMaxCols:11,maxChars:4000})).ndjson);
sheet.getRange('H1:L6').format.font={name:'Arial',size:11};
sheet.getRange('H1:L1').format.font={name:'Arial',size:11,bold:true};
sheet.getRange('H1:L6').format.columnWidthPx=160;
sheet.getRange('H1:L6').format.rowHeightPx=26;
sheet.getRange('H2:L6').setNumberFormat('0.000');
const preview=await workbook.render({sheetName:'Runtime comparison',range:'H1:L6',scale:1,format:'png'});
await fs.writeFile(path.join(here,'runtime_preview.png'),new Uint8Array(await preview.arrayBuffer()));
const matrix=sheet.getRange('A1:U6').values;
const encode=v=>{const s=String(v??'');return /[,"\r\n]/.test(s)?`"${s.replaceAll('"','""')}"`:s;};
const text=matrix.map(row=>row.map(encode).join(',')).join('\n')+'\n';
await fs.writeFile(path.join(root,'layout_switch_wins.csv'),text);
const imported=await Workbook.fromCSV(text,{sheetName:'CSV verification'});
const readback=imported.worksheets.getItem('CSV verification').getRange('A1:U6').values;
if(JSON.stringify(readback)!==JSON.stringify(matrix.map(row=>row.map(v=>String(v??'')))))throw new Error('CSV round-trip mismatch');
console.log(JSON.stringify(rows.map(r=>({case:r[0],AUTO_ms:r[7],best_fixed:r[16],speedup:r[17]})),null,2));
