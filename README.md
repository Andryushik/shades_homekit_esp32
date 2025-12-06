# Shades Controller (Seeed XIAO ESP32C6, Matter-ready)

Roller shades controller for Seeed Studio XIAO ESP32C6. Drives a 28BYJ‑48 stepper via ULN2003, provides calibration, web UI, OTA updates, and is prepared for Matter integration (HomeKit code removed).

## Features

- HomeKit accessory: `CurrentPosition`, `TargetPosition`, `PositionState`.
- Physical buttons: UP / DOWN; MAIN actions via simultaneous UP+DOWN.
- LED feedback: blinks in CALIBRATE, steady (LOW, active‑low) in NORMAL.
- Minimal persisted state (`/config.json`): currentStep, maxSteps, targetPositionValue, raw calibration points.
- Wi-Fi captive portal on first boot (AP: "Roller Shades Configuration").
- Continuous calibration jogging (constant speed; acceleration disabled).
- Web UI: movement control, presets (30% / 50% / 70%), calibration start/stop/save, safe reboot, factory reset.
- Safe reboot: saves current position, sets target to it, HTML page with auto redirect, then restart.

## Hardware

- MCU: Seeed Studio XIAO ESP32C6
- Stepper: 28BYJ‑48 — 12 V variant preferred for reliability and torque; ensure the ULN2003 board and power supply match the motor voltage.
- Driver: ULN2003 board.
- Buttons: 2 × tactile (wired to ground, `INPUT_PULLUP`, pressed = LOW).
- LED: onboard single LED (`LED_BUILTIN`).
- Power: stable supply matching motor voltage; ensure adequate current for both logic and motor.

Pin wiring (AccelStepper HALF4WIRE, coil order MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4 as used in code):

- MOTOR_IN1 → D1 (GPIO1)
- MOTOR_IN2 → D2 (GPIO2)
- MOTOR_IN3 → D3 (GPIO3)
- MOTOR_IN4 → D4 (GPIO4)
- BUTTON_UP_PIN → D8 (GPIO8)
- BUTTON_DOWN_PIN → D9 (GPIO9)
- LED_PIN → `LED_BUILTIN` (fallback GPIO15)

To reverse motor direction just flip the wiring order of the four ULN2003 inputs. For example: IN1→D1, IN2→D5, IN3→D6, IN4→D7 (no code changes are required when you flip the wiring).

Note: Adjust pins in `pins.h` if your wiring differs.

## First Setup

1. Power device; connect to AP "Roller Shades Configuration" and configure Wi‑Fi (captive portal via WiFiManager).
2. Calibrate via web UI or buttons.
3. Use web UI for control; Matter pairing to be added.

## Calibration

Goal: learn TOP and BOTTOM raw steps to derive travel (`maxSteps`). Uses constant speed (`CAL_SPEED`) with acceleration disabled for smoothness.

Flow:

1. Automatic entry if no calibration present; or hold BOTH ~5s.
2. Jog: press UP (or DOWN) to start continuous motion; press same again to stop (toggle behavior).
3. Save TOP: short BOTH press. Then jog to bottom.
4. Save BOTTOM: short BOTH press again → success if travel ≥ `MIN_TRAVEL = 4096` steps.
5. On success: rebase (TOP=0, BOTTOM=`maxSteps`), persist, exit to NORMAL.

Tips:

- LED blinks while in calibration.
- Stop motion before saving (for precision).
- If "Travel too small" message appears, increase physical travel and retry.

## Manual Control (NORMAL mode)

- UP: set target to 100% (fully open).
- DOWN: set target to 0% (closed).
- MAIN (UP+DOWN): short press STOP (target := current position, position state STOPPED); ~5s enter CALIBRATE; ~10s factory reset.

## Configuration Persistence

Saved keys: `currentStep`, `maxSteps`, `targetPositionValue`, `rawUpStep`, `rawDownStep`.

Save triggers:

- Movement completes in NORMAL mode.
- Successful calibration.
- Just before safe reboot.

## Reset & Safe Reboot

- Factory reset (MAIN ~10s): clears Wi-Fi, SPIFFS config file, HomeKit pairing; restarts into captive portal.
- Safe Reboot (web `/reboot`): stops motion, sets target=current position, saves config, serves HTML "Rebooting" page (auto redirect ~15s), restarts.

## Web UI

Root (`/`): Mode, current step, maxSteps, percent position, last message, buttons: Up / Down / Stop, Presets 30/50/70, Start/Exit Calibration, Save Top/Bottom, Safe Reboot, Factory Reset.

Status polling: JS fetches `/status` every 200 ms. Stop button blinks while position changes. Messages display movement or calibration feedback (errors highlighted).

Endpoints (POST unless marked GET):

- `/cal/start`, `/cal/stop`
- `/cal/up/start`, `/cal/down/start` (toggle continuous jog in CALIBRATE)
- `/preset/30`, `/preset/50`, `/preset/70` (move to 30% / 50% / 70%)
- `/cal/hold/stop` (stop jog OR STOP motion in NORMAL)
- `/cal/saveTop`, `/cal/saveBottom`
- `/reboot` (safe reboot)
- `/factory` (factory reset)
- `/status` (GET): JSON `{ currentStep, maxSteps, mode, position, msg }`

Writes to SPIFFS limited to essential moments (movement completion, calibration success, reboot).

You can reach the web UI either by entering the device IP in your browser (for example `http://192.168.x.y/`) or, once mDNS is active, via `http://roller_shades.local/` on the same LAN.

## 3D Files

Smart roller shades printable parts and enclosure:

- <https://www.printables.com/model/1505710-smart-roller-shades-esp8266-homekit>

## Software & Build

- Board: `esp32:esp32:XIAO_ESP32C6`
- Libraries: WiFiManager (ESP32), ArduinoJson v7, AccelStepper, EasyButton, FS/LittleFS.
- Serial: 115200 baud.

Build & upload via Arduino CLI:

```zsh
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 .
arduino-cli upload -p /dev/tty.usbmodemXXXX --fqbn esp32:esp32:XIAO_ESP32C6 .
```

OTA update (device hostname `roller_shades`):

```zsh
python3 $HOME/Library/Arduino15/packages/esp32/tools/espota/*/espota.py \
  -i roller_shades.local -p 3232 -f /path/to/firmware.bin
```

### Constants & Configuration

Timing constants (configurable in `Globals.h`):

- `BUTTON_IGNORE_WINDOW_MS = 10000` (10s button ignore after boot)
- `MAIN_LONG_PRESS_CAL_MS = 5000` (5s press for calibration mode)
- `MAIN_LONG_PRESS_RESET_MS = 10000` (10s press for factory reset)
- `PRESET_DEFER_WINDOW_MS = 150` (debounce for preset actions)
- `LED_BLINK_INTERVAL_MS = 400` (LED blink rate)

### Debugging

- Serial debug: 115200 baud (enable with `-DSHADES_DEBUG` flag).
- Build with debug:

```zsh
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 \
  --build-property build.extra_flags="-DSHADES_DEBUG" .

## Matter Integration (ESP32C6)
- Seeed guide: https://wiki.seeedstudio.com/xiao_esp32_matter_env/
- Espressif Matter setup: use ESP-IDF with Matter examples; Arduino integration typically wraps Matter via components.
- Plan: add a Matter Window Covering cluster to control target percentage; wire it to `targetPercent` and report position via `getCurrentPosition()`.
- Until Matter is integrated, use the web UI for control.
```

## Inspiration

- <https://www.thingiverse.com/thing:2392856/>

## License

Non‑commercial use (as‑is). HomeKit library subject to Apple terms for non‑commercial use.
