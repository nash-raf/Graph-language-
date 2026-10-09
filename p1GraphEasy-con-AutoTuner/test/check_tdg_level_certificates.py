#!/usr/bin/env python3
"""Compatibility entrypoint; implementation is in pdg_tdg_verification."""
from pathlib import Path
import runpy
import sys

target = Path(__file__).resolve().parent / "pdg_tdg_verification" / "check_tdg_level_certificates.py"
sys.argv[0] = str(target)
runpy.run_path(str(target), run_name="__main__")
