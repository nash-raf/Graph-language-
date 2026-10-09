#!/usr/bin/env bash

sgpl_runtime_test_temp() {
  mkdir -p "$ROOT/build_tdg_validation"
  mktemp -d "$ROOT/build_tdg_validation/runtime_test_tmp.XXXXXX"
}

sgpl_runtime_test_cleanup() {
  local resolved
  resolved="$(realpath -- "$1")"
  case "$resolved" in
    "$ROOT"/build_tdg_validation/runtime_test_tmp.*) rm -rf -- "$resolved" ;;
    *) echo "refusing to remove unexpected runtime test path: $resolved" >&2; return 1 ;;
  esac
}
