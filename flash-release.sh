#!/usr/bin/env bash
# Compile WITHOUT SHADES_DEBUG and OTA-flash the device.
# No DPRINTs, no telnet console, no heartbeats — production firmware.
#
# Usage:
#   ./flash-release.sh                          # default port: 192.168.2.202
#   ./flash-release.sh RollerShades.local       # mDNS
#   ./flash-release.sh 192.168.2.225            # different IP

set -e

PORT="${1:-192.168.2.202}"
FQBN="esp32:esp32:XIAO_ESP32C6:PartitionScheme=min_spiffs"
OTA_PASSWORD="28142814"
BUILD_DIR="./build/release"

cd "$(dirname "$0")"

echo "==> compile (release)"
arduino-cli compile --fqbn "$FQBN" \
  --output-dir "$BUILD_DIR" \
  .

echo "==> OTA upload to $PORT"
arduino-cli upload --fqbn "$FQBN" \
  --input-dir "$BUILD_DIR" \
  --protocol network \
  --port "$PORT" -F password="$OTA_PASSWORD" \
  .
