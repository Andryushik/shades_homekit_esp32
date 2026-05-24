#!/usr/bin/env bash
# Telnet debug console capturer with auto-reconnect.
# Writes timestamped + tagged lines to shades.log next to this script.
#
# Usage:
#   ./log-shades.sh                  # default: 192.168.2.202 (RollerShades)
#   ./log-shades.sh <host-or-ip>     # arbitrary
#
# To run detached:
#   nohup ./log-shades.sh > /dev/null 2>&1 &
#   disown
#   tail -f shades.log

PORT=23
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG="$SCRIPT_DIR/shades.log"
HOST="${1:-192.168.2.202}"
TAG="shades"

while true; do
  printf '[%s][%s] === connecting to %s:%s ===\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$TAG" "$HOST" "$PORT" >> "$LOG"
  # -w 15: drop the connection if no data for 15 s. Device sends a heartbeat
  # every 5 s, so 15 s of silence = half-open or dead peer → force reconnect.
  nc -w 15 "$HOST" "$PORT" 2>&1 | while IFS= read -r line; do
    printf '[%s][%s] %s\n' "$(date '+%H:%M:%S')" "$TAG" "$line" >> "$LOG"
  done
  printf '[%s][%s] === disconnected, reconnecting in 3s ===\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$TAG" >> "$LOG"
  sleep 3
done
