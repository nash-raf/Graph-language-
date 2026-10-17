#!/usr/bin/env python3
"""Recheck saved executables without changing or repeating timed measurements."""
from run_layout_switch_suite import CONFIGS, OUT
from layout_switch_benchmark import MODES, audit, build, compile_case, source

if __name__=='__main__':
    for config in CONFIGS:
        work,objs=build(OUT)
        name,src,expected=source(OUT,**config)
        schedules={m:compile_case(OUT,work,objs,name,src,m)[1] for m in MODES}
        audit(OUT,config,work,objs,name,expected,schedules)
