#!/usr/bin/env bash
# calibrate_hw.sh — Turbo-enabled hardware calibration with settings save/restore.
#
# Usage:
#   sudo ./calibrate_hw.sh
#
# Sets performance governor + max turbo for stable calibration, runs hw_calib_bench,
# then restores original settings.  Does NOT overwrite an existing hw_calib.json
# unless --force is passed (or the file doesn't exist).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

CALIB_DIR="${HOME}/.config/sgpl"
CALIB_FILE="${CALIB_DIR}/hw_calib.json"

# Check if calibration already exists
FORCE="${1:-}"
if [[ -f "$CALIB_FILE" && "$FORCE" != "--force" ]]; then
  echo "Calibration already exists at ${CALIB_FILE}"
  echo "Pass --force to re-calibrate."
  exit 0
fi

# Build hw_calib_bench if needed
if [[ ! -x ./hw_calib_bench ]]; then
  echo "Building hw_calib_bench ..."
  gcc -O2 -o hw_calib_bench hw_calib_bench.c
fi

# --- Save original CPU settings ---
GOV_SAVED="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "unknown")"
MIN_SAVED="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq 2>/dev/null || echo "")"
MAX_SAVED="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null || echo "")"
NO_TURBO_SAVED="$(cat /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null || echo "")"

restore_settings() {
  echo "Restoring original CPU settings ..."
  if [[ -n "$GOV_SAVED" && "$GOV_SAVED" != "unknown" ]]; then
    echo "$GOV_SAVED" | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null 2>&1 || true
  fi
  if [[ -n "$MIN_SAVED" ]]; then
    for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_min_freq; do
      echo "$MIN_SAVED" > "$cpu" 2>/dev/null || true
    done
  fi
  if [[ -n "$MAX_SAVED" ]]; then
    for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do
      echo "$MAX_SAVED" > "$cpu" 2>/dev/null || true
    done
  fi
  if [[ -n "$NO_TURBO_SAVED" ]]; then
    echo "$NO_TURBO_SAVED" > /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null || true
  fi
}

trap restore_settings EXIT

# --- Set performance governor + max turbo ---
echo "Setting performance governor with turbo enabled ..."
echo "performance" | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null

# Read hardware limits
CPUINFO_MIN="$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq 2>/dev/null || echo "400000")"
CPUINFO_MAX="$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq 2>/dev/null || echo "4400000")"
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_min_freq; do
  echo "$CPUINFO_MAX" > "$cpu" 2>/dev/null || true
done
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do
  echo "$CPUINFO_MAX" > "$cpu" 2>/dev/null || true
done

# Ensure turbo is enabled
if [[ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]]; then
  echo 0 > /sys/devices/system/cpu/intel_pstate/no_turbo
fi

# Let the machine cool for 15 seconds (fans spin down, die cools)
echo "Waiting 15 s for the die to cool ..."
sleep 15

# --- Run calibration ---
mkdir -p "$CALIB_DIR"
echo "Running hw_calib_bench ..."
./hw_calib_bench > "$CALIB_FILE"

echo ""
echo "=== New calibration saved to ${CALIB_FILE} ==="
cat "$CALIB_FILE"

echo ""
echo "Settings will be restored on exit."
