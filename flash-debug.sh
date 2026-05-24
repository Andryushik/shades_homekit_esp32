#!/usr/bin/env bash
# Compile with SHADES_DEBUG and OTA-flash the device.
# DPRINTs + telnet console on port 23 + heartbeats are active in this build.
#
# Usage:
#   ./flash-debug.sh                          # default port: 192.168.2.202
#   ./flash-debug.sh RollerShades.local       # mDNS (survives DHCP changes)
#   ./flash-debug.sh 192.168.2.225            # different IP

set -e

PORT="${1:-192.168.2.202}"
FQBN="esp32:esp32:XIAO_ESP32C6:PartitionScheme=min_spiffs"
OTA_PASSWORD="28142814"
BUILD_DIR="./build/debug"

cd "$(dirname "$0")"

echo "==> compile (debug)"
arduino-cli compile --fqbn "$FQBN" \
  --build-property "compiler.cpp.extra_flags=-DSHADES_DEBUG" \
  --output-dir "$BUILD_DIR" \
  .

echo "==> OTA upload to $PORT"
arduino-cli upload --fqbn "$FQBN" \
  --input-dir "$BUILD_DIR" \
  --port "$PORT" -F password="$OTA_PASSWORD" \
  .
