#!/usr/bin/env python3
"""Linux integration checks for benchmark interruption and checkpoint recovery."""
import importlib.util, json, os, signal, subprocess, sys, tempfile, time, unittest
from pathlib import Path

path=Path(sys.argv.pop(1)).resolve() if len(sys.argv)>1 else Path(__file__).with_name('run_e2e150.py')
spec=importlib.util.spec_from_file_location('runner',path);runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
ROOT=path.parent

class RunnerIntegration(unittest.TestCase):
    def test_journal_recovers_only_last_incomplete_append(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'journal.jsonl';p.write_bytes(b'{"sample":0}\n{"sample":')
            self.assertEqual(runner.journal(p),[{'sample':0}])
            runner.append(p,{'sample':1});self.assertEqual(len(runner.journal(p)),2)
            p.write_bytes(b'{"sample":0}\nbroken\n{"sample":2}\n')
            with self.assertRaises(RuntimeError):runner.journal(p)

    def test_timer_covers_program_startup_and_exit(self):
        with tempfile.TemporaryDirectory() as d:
            d=Path(d)
            result=runner.Launcher(ROOT).run([sys.executable,'-c','import time; time.sleep(.2)'],d,os.environ.copy(),d/'run.log',5,d/'stamp')
            self.assertEqual(result['exit_code'],0);self.assertGreaterEqual(result['seconds'],.2)
            self.assertLess(result['seconds'],3);self.assertEqual(result['child_exit'],0)

    def test_timeout_kills_descendants_in_exact_scope(self):
        with tempfile.TemporaryDirectory() as d:
            d=Path(d);pidfile=d/'child.pid'
            code="import subprocess,time,pathlib; p=subprocess.Popen(['sleep','60']); pathlib.Path("+repr(str(pidfile))+").write_text(str(p.pid)); time.sleep(60)"
            result=runner.Launcher(ROOT).run([sys.executable,'-c',code],d,os.environ.copy(),d/'timeout.log',1,d/'stamp')
            self.assertEqual(result['exit_code'],124);self.assertTrue(result['timeout'])
            self.assertTrue(pidfile.exists());pid=int(pidfile.read_text())
            for _ in range(30):
                status=Path(f'/proc/{pid}/stat')
                if not status.exists() or status.read_text().split()[2]=='Z':break
                time.sleep(.1)
            else:self.fail('Timed-out descendant remains runnable')
            active=subprocess.run(['systemctl','--user','is-active',result['unit']],capture_output=True,text=True)
            self.assertNotEqual(active.stdout.strip(),'active')

    def test_interrupted_real_run_resumes_without_duplicate_samples(self):
        with tempfile.TemporaryDirectory(prefix='e2e150-resume-') as d:
            d=Path(d);out=d/'results'
            cmd=[sys.executable,'-u',str(path),'--graph','protein/DD_g714.edges',
                 '--algorithms','bfs','--frameworks','ligra','--repeats','10',
                 '--output',str(out),'--resume']
            with open(d/'first.log','w') as log:
                process=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
                until=time.monotonic()+30
                while time.monotonic()<until:
                    p=out/'launches.jsonl'
                    if p.exists() and len(p.read_text().splitlines())>=3:break
                    if process.poll() is not None:self.fail('Runner exited before interruption test: '+(d/'first.log').read_text())
                    time.sleep(.01)
                else:
                    process.kill();process.wait();self.fail('Runner did not checkpoint within 30s')
                process.send_signal(signal.SIGINT);self.assertEqual(process.wait(timeout=20),130)
            before=runner.journal(out/'launches.jsonl')
            self.assertLess(sum(e['phase']=='measure' for e in before),10)
            with open(d/'resume.log','w') as log:
                process=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=60)
            self.assertEqual(process.returncode,0,(d/'resume.log').read_text())
            after=runner.journal(out/'launches.jsonl')
            samples=[e['sample'] for e in after if e['phase']=='measure' and not e['exit_code']]
            self.assertEqual(sorted(samples),list(range(10)))
            self.assertEqual(sum(e['phase']=='resume_warmup' for e in after),1)
            cases=runner.journal(out/'cases.jsonl');self.assertEqual(cases[0]['status'],'ok')

if __name__=='__main__':unittest.main()
