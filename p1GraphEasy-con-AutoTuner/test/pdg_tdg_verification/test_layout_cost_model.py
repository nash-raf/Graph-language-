#!/usr/bin/env python3
"""Check the sole layout model in Python and the actual LLVM pass (Linux/WSL)."""
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
import cost_model as cm


class LayoutModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix="ars-layout-model-")
        cls.addClassCleanup(cls.tmp.cleanup)
        tmp = Path(cls.tmp.name)
        cls.calibration = tmp / "calibration.json"
        cls.calibration.write_text(json.dumps({
            "L": 64, "t": 2, "T": 3, "R": 10000,
            "LLC": 12582912,
            "class_rates": {"seq": [1, 2, 4], "rmw": [5, 7, 11],
                            "rand": [13, 17, 19], "set_iter": [3, 6, 9]},
        }))
        source = tmp / "probe.cpp"
        source.write_text('#include "' + (ROOT / "AutoTunerPass.cpp").as_posix() + '"\n' + r'''
#include <iostream>
int main() {
  HwCalib hw = loadHwCalib();
  double tiers[12] = {};
  tiers[4] = .5; tiers[5] = .75;
  tiers[7] = .25; tiers[8] = .75;
  std::cout.precision(17);
  for (int layout = 0; layout < 4; ++layout)
    std::cout << traversalCost(layout, 128, 1024, hw) << "\n";
  for (const double *payload : {tiers, static_cast<double *>(nullptr)}) {
    std::cout << insertCost(LAYOUT_CSR, 128, 1024, .25, .75, hw,
                            payload, payload) << "\n";
    std::cout << insertCost(LAYOUT_BCSR, 128, 1024, .25, .75, hw,
                            payload, payload) << "\n";
    hw.R = 1234567;
  }
  std::cout << insertCost(LAYOUT_PCSR, 128, 1024, .25, .75, hw) << "\n";
  std::cout << insertCost(LAYOUT_SET, 128, 1024, .25, .75, hw) << "\n";
}
''')
        flags = shlex.split(subprocess.check_output([
            os.environ.get("LLVM_CONFIG", "llvm-config-20"), "--cxxflags",
            "--ldflags", "--libs", "core", "irreader", "analysis", "passes",
            "--system-libs"], text=True))
        cls.probe = tmp / "probe"
        subprocess.run([os.environ.get("CXX", "g++"), "-O0", "-iquote",
                        str(ROOT), str(source), *flags, "-o", str(cls.probe)],
                       check=True)

    def setUp(self):
        self.original = {name: getattr(cm, name) for name in
                         ("t", "T", "L", "LLC", "_CLASS_RATES", "_CUR_GRAPH")}
        cm.t, cm.T, cm.L, cm.LLC = 2, 3, 64, 12582912
        cm._CUR_GRAPH = None
        cm._CLASS_RATES = {"scan": (1, 2, 4), "move": (5, 7, 11),
                           "brow": (5, 7, 11), "struct": (13, 17, 19),
                           "set_iter": (3, 6, 9)}
        self.tiers = {"move": (0, .5, .75), "brow": (0, .25, .75)}

    def tearDown(self):
        for name, value in self.original.items():
            setattr(cm, name, value)

    def test_calibrated_equations_in_actual_pass(self):
        # Fixed costs for the fixture, including both directed insertions.
        expected = [1280, 1664, 50048, 410112, 239, 2126, 359, 2894, 13, 29]
        for retired in ("hybrid", "legacy", "aware", "class_tier"):
            env = dict(os.environ, AUTOTUNER_HW_CALIB=str(self.calibration),
                       AUTOTUNER_CACHE_MODEL=retired)
            actual = subprocess.check_output([str(self.probe)], env=env, text=True)
            self.assertEqual([float(x) for x in actual.split()], expected)

    def test_python_preserves_current_metadata_equations(self):
        self.assertEqual(cm.insert_cost_csr(128, 1024, .25, self.tiers), 239)
        self.assertEqual(cm.insert_cost_bcsr(128, 1024, .75, self.tiers), 2126)
        self.assertEqual(cm.traverse_cost_set(128, 1024), 410112)

    def test_missing_metadata_uses_current_dram_rates(self):
        self.assertEqual(cm.insert_cost_csr(128, 1024, .25), 359)
        self.assertEqual(cm.insert_cost_bcsr(128, 1024, .75), 2894)
        # The prefix remains; a zero shift costs no move or relocation.
        self.assertEqual(cm.insert_cost_csr(128, 1024, 0), 183)
        self.assertEqual(cm.insert_cost_bcsr(128, 1024, 0), 782)

    def test_retired_python_modes_cannot_be_selected(self):
        cm.set_cache_model("hybrid")
        for retired in ("legacy", "aware", "class_tier"):
            with self.assertRaisesRegex(ValueError, "Retired cost model"):
                cm.set_cache_model(retired)


if __name__ == "__main__":
    unittest.main(verbosity=2)
