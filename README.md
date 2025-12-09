# Roller Shades Controller (ESP32 + HomeKit)

ESP32-based roller shades controller with native HomeKit support via HomeSpan.

## Features

- **HomeKit Native** - Direct HomeKit integration via HomeSpan (no bridge needed)
- **WindowCovering Service** - Proper shades/blinds control in Home app
- **WiFi Configuration** - WiFiManager captive portal for easy setup
- **Physical Controls** - Two-button interface for manual operation
- **Web Interface** - Built-in web UI for control and configuration
- **Calibration** - Automatic range calibration with position persistence
- **Smooth Motion** - AccelStepper library for smooth acceleration/deceleration
- **Status LED** - Visual feedback for WiFi, calibration, and movement

## Hardware Requirements

- **ESP32-C6** recommended (or any ESP32 variant: ESP32, ESP32-S3, ESP32-C3)
- **28BYJ-48 Stepper Motor** with ULN2003 driver
- **Two Push Buttons** (UP and DOWN)
- **LED** (optional status indicator)
- **Power Supply** - 5V for motor and ESP32

## Pin Configuration

Default pins (configurable in `pins.h`):

- Motor: IN1=GPIO1, IN2=GPIO2, IN3=GPIO21, IN4=GPIO22
- Buttons: UP=GPIO19, DOWN=GPIO20
- LED: GPIO15 (active-low with PWM brightness control)

**XIAO ESP32C6 pin map:**

- D1=GPIO1, D2=GPIO2, D3=GPIO21, D4=GPIO22
- D8=GPIO19 (SCK), D9=GPIO20 (MISO)
- LED=GPIO15 (onboard LED)

## Installation

### 1. Arduino IDE Setup

```bash
# Install ESP32 board support (v3.3.4+)
# Add to Board Manager URLs:
# https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

### 2. Install Libraries

```bash
arduino-cli lib install AccelStepper
arduino-cli lib install EasyButton
arduino-cli lib install ArduinoJson
arduino-cli lib install WiFiManager
arduino-cli lib install HomeSpan
```

### 3. Upload Sketch

1. Select board: ESP32 → XIAO_ESP32C6 (or your ESP32 variant)
2. Select partition scheme: **No OTA** (2MB APP/2MB SPIFFS)
3. Upload `shades_homekit_esp32.ino`

### 4. WiFi Setup

1. On first boot, ESP32 creates WiFi network: **"RollerShades"**
2. Connect to it from phone/computer
3. Captive portal opens automatically
4. Select your WiFi network and enter password
5. Device will reboot and connect

### 5. HomeKit Pairing

1. Open Home app on iPhone/iPad
2. Tap **+** → **Add Accessory** → **More Options**
3. Select **"Roller Shades"** from the list
4. Enter setup code: **466-37-726**
5. Assign to room and done!

## Usage

### Physical Buttons

- **UP** - Open shades to 100%
- **DOWN** - Close shades to 0%
- **UP + DOWN (short press)** - Stop motion
- **UP + DOWN (5 seconds)** - Enter calibration mode
- **UP + DOWN (10 seconds)** - Factory reset (WiFi + HomeKit + config)

### LED Status Indicators

- **Fast blink (200ms)** - Not calibrated or WiFi disconnected
- **Slow blink (400ms)** - Calibration mode active
- **Bright (100%)** - Moving
- **Dim (1%)** - Idle/stopped
- **Full on** - WiFi captive portal active

### Calibration Process

1. **Long-press both buttons (5s)** - Enter calibration (LED blinks)
2. **Use UP/DOWN** - Move to fully open (top) position
3. **Short press both buttons** - Save top position (LED flashes 5x)
4. **Use UP/DOWN** - Move to fully closed (bottom) position
5. **Short press both buttons** - Save bottom and exit (LED flashes 5x)
6. LED returns to normal mode, calibration complete

### Web Interface

Access via `http://rollershades.local` or IP address:

- **Control Panel** - Open/Close/Stop, 30%/50%/70% presets
- **Current Position** - Real-time percentage display
- **Calibration** - Enter/exit calibration, save positions
- **HomeKit Pairing** - Shows setup code 466-37-726
- **System** - Reboot or factory reset

### HomeKit Control

- **On** - Open to 100%
- **Off** - Close to 0%
- **Slider** - Set to any position 0-100%
- **Siri** - "Open the roller shades", "Close the shades", "Set shades to 50%"
- **Automations** - Schedule, scenes, triggers

## Configuration

### Motor Settings (`shades_homekit_esp32.ino`)

```cpp
const float SPEED_MAX = 900.0f;      // steps/second
const float ACCEL = 300.0f;           // steps/second²
const float CAL_SPEED = 300.0f;       // calibration speed
const int32_t HOLD_TORQUE_MS = 3000;  // motor hold time (or -1 for always on)
const int MIN_TRAVEL = 4096;          // minimum calibration distance
```

### Pin Assignment (`pins.h`)

```cpp
#define MOTOR_IN1 1
#define MOTOR_IN2 2
#define MOTOR_IN3 21
#define MOTOR_IN4 22
#define BUTTON_UP_PIN 19
#define BUTTON_DOWN_PIN 20
#define LED_PIN 15
```

### HomeKit Settings (`homespan_bridge.cpp`)

- Setup code: `466-37-726` (default, can be changed in code)
- Device name: "Roller Shades"
- Category: Window Coverings

## Troubleshooting

### Shades Don't Move

- Check motor wiring (IN1-IN4)
- Verify 5V power supply is connected
- Ensure calibration is complete
- Check serial monitor for errors

### WiFi Connection Issues

- Reset device (power cycle)
- Captive portal will re-appear
- If not, factory reset (10s button hold)
- Check router 2.4GHz band is enabled

### HomeKit Pairing Fails

- Factory reset device (10s button hold)
- Remove from Home app if present
- Reboot device and re-pair
- Ensure iPhone/iPad on same WiFi network
- Try setup code again: 466-37-726

### Position Drift

- Re-calibrate using buttons or web UI
- Ensure motor has enough power
- Check `HOLD_TORQUE_MS` setting
- Verify stepper is firmly attached

### LED Not Working

- Check `LED_PIN` setting matches your board
- LED is active-low on XIAO ESP32C6
- LED is optional for operation

## Technical Details

### Libraries Used

- **HomeSpan 2.1.6+** - Native HomeKit integration
- **AccelStepper 1.64+** - Smooth motor control
- **EasyButton 2.0.3+** - Debounced button input
- **WiFiManager 2.0.17+** - WiFi setup
- **ArduinoJson 7.4+** - Config storage

### Storage

- **SPIFFS** - Configuration persistence
- **NVS** - HomeKit pairing data
- **Config file** - `/config.json` (current position, calibration)

### Partition Scheme

Recommended: **No OTA (2MB APP/2MB SPIFFS)**

- Sketch uses ~1.4MB (45% of 3MB, fits easily in 2MB)
- 2MB SPIFFS provides ample storage for config
- OTA updates not needed for this application

## Development

### Serial Monitor

Enable debug output by defining `SHADES_DEBUG`:

```cpp
// In Globals.h or compile flags
#define SHADES_DEBUG
```

### Build Commands

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6:PartitionScheme=no_ota .

# Upload
arduino-cli upload --fqbn esp32:esp32:XIAO_ESP32C6:PartitionScheme=no_ota -p /dev/ttyUSB0 .
```

## 3D Printable Parts

Smart roller shades enclosure and mounting hardware:

- <https://www.printables.com/model/1505710-smart-roller-shades-esp8266-homekit>

## License

MIT License

## Credits

- [HomeSpan](https://github.com/HomeSpan/HomeSpan) - Excellent HomeKit library for ESP32
- [WiFiManager](https://github.com/tzapu/WiFiManager) - Easy WiFi configuration
- [AccelStepper](http://www.airspayce.com/mikem/arduino/AccelStepper/) - Smooth motor control

## Support

For issues or questions:

- Check serial monitor output (115200 baud)
- Review HomeSpan documentation
- Ensure latest library versions installed
