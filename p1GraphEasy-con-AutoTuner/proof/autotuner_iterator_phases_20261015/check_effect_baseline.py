"""Compare the existing effect-lowering assertion before/after this change."""
import importlib.util
import os
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("frequency", ROOT / "test/pdg_tdg_verification/test_autotuner_frequency.py")
frequency = importlib.util.module_from_spec(spec)
spec.loader.exec_module(frequency)
source = HERE / "effect_one.input.ll"
source.write_text(frequency.lowered_loop(1))
before = HERE / "AutoTunerPass.before.cpp"
before.write_text(subprocess.check_output(["git", "show", "HEAD:p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp"], cwd=ROOT, text=True))
flags = shlex.split(subprocess.check_output(["llvm-config-20", "--cxxflags", "--ldflags", "--libs", "all", "--system-libs"], text=True))
env = {k: v for k, v in os.environ.items() if not k.startswith(("SGPL_", "GRAPH_", "AUTOTUNER_"))}
env.update(AUTOTUNER_FORCE_LAYOUT="CSR", SGPL_FRONTIER_STRICT="1", GRAPH_FRONTIER_STATS="1")
for name, tuner in (("before", before), ("after", ROOT / "AutoTunerPass.cpp")):
    exe = HERE / f"effect_{name}"
    subprocess.run(["g++", "-O1", "-iquote", str(ROOT), str(ROOT / "test/pdg_tdg_verification/autotuner_frequency_driver.cpp"),
                    str(tuner), str(ROOT / "build_lowmem/graph_frontier_lowering.o"), *flags, "-o", str(exe)], check=True)
    result = subprocess.run([str(exe), str(source), "--lower"], env=env, capture_output=True, text=True, check=True)
    (HERE / f"effect_{name}.ll").write_text(result.stdout)
    (HERE / f"effect_{name}.log").write_text(result.stderr)
    print(name, "clean_cut_call=", "call i32 @autograph_build_clean_cut" in result.stdout,
          "execute_call=", "call i32 @autograph_frontier_execute" in result.stdout, flush=True)
