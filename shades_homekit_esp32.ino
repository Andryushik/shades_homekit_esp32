#include <Arduino.h>
#include "HomeSpan.h"
#include <nvs_flash.h>
#include "Helper.h"
#include "pins.h"
#include "Buttons.h"
#include "ButtonActions.h"
#include <AccelStepper.h>
#include "Globals.h"
#include "web.h"
#include "LedControl.h"
#include "HomeKitShade.h"
#include "HomeSpanConfig.h"
#include <WiFi.h>
#include "RemoteLog.h"
#include "Diagnostics.h"

// Speed/settings constants
const float SPEED_MAX = 900.0f; // steps/s
const float ACCEL = 300.0f;     // steps/s^2
const float CAL_SPEED = 300.0f; // steps/s during calibration (continuous)
// HOLD_TORQUE_MS semantics:
//   0   -> disable coils immediately after stop
//  >0   -> keep coils energized for that many milliseconds, then disable
//  -1   -> never disable (always hold torque) WARNING: motor/driver will stay warm
const int32_t HOLD_TORQUE_MS = 3000; // change to -1 for infinite hold
const int MIN_TRAVEL = 4096;         // minimum calibration travel 1 full rotation (steps)

// 28BYJ-48 via ULN2003 using HALF4WIRE; coil order MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4
AccelStepper stepper(AccelStepper::HALF4WIRE, MOTOR_IN1, MOTOR_IN3, MOTOR_IN2, MOTOR_IN4);

Helper helper;

static const char *HOSTNAME = "RollerShades";

// Centralized runtime state (see `Globals.h` for field docs)
ShadesState state = {
    .currentMode = NORMAL,
    .currentCalibrationStep = NONE,
    .confirmBlinkActive = false,
    .exitCalibrationAfterBlink = false,
    .currentStep = 0,
    .maxSteps = 0,
    .upStep = 0,
    .downStep = 0,
    .targetPercent = 0,
    .calJogDir = 0,
    .calRequireRelease = false,
    .holdingActive = false,
    .lastBothPressed = false,
    .bothPressStart = 0,
    .mainLong5Handled = false,
    .mainLong10Handled = false,
    .startupTime = 0,
    .lastMovementTime = 0,
    .lastMessage = String()};

int getCurrentPosition();
bool loadConfig();
bool saveConfig();
void enableCalibrationMode();
void properLedDisplay();
void handleEngineControllerActivity();
void shadesControl();
void motorTick();

void setup()
{
  Serial.begin(115200);
  SERIAL_DEBUG_INIT();
  DPRINTLN("=== TEST BUILD " __DATE__ " " __TIME__ " ===");
  diag_init();
  diag_installWiFiEventLogger();

  // Set DHCP/mDNS hostname so routers show a friendly name
  WiFi.setHostname(HOSTNAME);
  // WiFi.setSleep(false) — modem-sleep disabled. We get lower-latency HomeKit
  // (no "Not Responding" jitter from DTIM-delayed unicast RX). The extra
  // ~30-50 mA continuous radio draw is negligible on the 12V supply here.
  WiFi.setSleep(false);

  Led::begin();
  // BUTTON_MAIN is simulated by both UP+DOWN pressed
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  state.startupTime = millis();

  // Initialize LittleFS for config storage
  if (helper.begin())
  {
    DPRINTLN("LittleFS initialized successfully");
  }
  else
  {
    DPRINTLN("ERROR: Failed to initialize LittleFS!");
  }

  loadConfig();
  if (state.maxSteps == 0)
    enableCalibrationMode();

  // Initialize stepper with normal motion profile
  stepper.setMaxSpeed(SPEED_MAX);
  stepper.setAcceleration(ACCEL);
  // Optional: tune pulse width for ULN2003 if needed (default is usually fine)
  // stepper.setMinPulseWidth(2);
  stepper.setCurrentPosition(state.currentStep);

  // Initialize HomeSpan (all configuration in HomeSpanConfig.cpp)
  homeSpanSetup();

  Buttons::init();
  DPRINTLN("Setup complete, entering loop...");
}

// Motor tick — call as often as possible to prevent step starvation on
// the single-core ESP32-C6. Safe to call multiple times per loop;
// stepper.run()/runSpeed() only steps when timing is due.
void motorTick()
{
  static bool wasCalibrating = false;

  if (state.currentMode == CALIBRATE)
  {
    wasCalibrating = true;

    bool up = (state.calJogDir < 0);
    bool down = (state.calJogDir > 0);
    if (state.calRequireRelease)
    {
      if (!up && !down)
        state.calRequireRelease = false;
    }

    if (!state.calRequireRelease)
    {
      if (up && !down)
      {
        stepper.setSpeed(-CAL_SPEED);
        stepper.runSpeed();
        state.lastMovementTime = millis();
      }
      else if (down && !up)
      {
        stepper.setSpeed(CAL_SPEED);
        stepper.runSpeed();
        state.lastMovementTime = millis();
      }
      else
      {
        stepper.setSpeed(0.0f);
      }
    }
  }
  else
  {
    if (wasCalibrating)
    {
      stepper.setAcceleration(ACCEL);
      stepper.setMaxSpeed(SPEED_MAX);
      wasCalibrating = false;
      state.calJogDir = 0;
    }

    if (stepper.distanceToGo() != 0)
    {
      stepper.enableOutputs();
      state.lastMovementTime = millis();
    }
    stepper.run();
  }

  // Keep state in sync with actual motor position
  state.currentStep = stepper.currentPosition();
}

void loop()
{
  static unsigned long lastHousekeeping = 0;
  static bool webServerStarted = false;
  static unsigned long lastHeartbeat = 0;
  const unsigned long HOUSEKEEP_MS = 100;
  const unsigned long HEARTBEAT_MS = 5000;
  const unsigned long SLOW_LOOP_THRESHOLD_MS = 50;

  unsigned long loopStart = millis();

  // Motor is highest priority — step before anything can block
  motorTick();

  // HomeSpan (potentially blocking: TLS, HAP notifications)
  homeSpan.poll();
  motorTick(); // recover after HomeSpan

  // Start/restart web server when WiFi connects
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected && !webServerStarted)
  {
    webBegin();
#ifdef SHADES_DEBUG
    rlog.begin(); // telnet debug console on port 23 — only after WiFi is up so LWIP is ready
#endif
    webServerStarted = true;
    DPRINTF("Web server started at http://%s:8080\n", WiFi.localIP().toString().c_str());
  }
  else if (!wifiConnected && webServerStarted)
  {
    webServerStarted = false;
    DPRINTLN("WiFi disconnected, web server will restart on reconnect");
  }

  // Buttons + target update (fast)
  Buttons::loop();
  shadesControl();
  motorTick(); // step toward new target immediately

  // UI + housekeeping
  properLedDisplay();
  if (millis() - lastHousekeeping >= HOUSEKEEP_MS)
  {
    handleEngineControllerActivity();
    lastHousekeeping = millis();
  }

  // Web server (potentially blocking: HTTP request/response)
  webLoop();
#ifdef SHADES_DEBUG
  rlog.loop(); // accept/manage telnet debug clients
#endif
  motorTick(); // recover after web server

  // Slow-loop detector + heartbeat telemetry
  unsigned long loopEnd = millis();
  unsigned long loopDuration = loopEnd - loopStart;
  if (loopDuration >= SLOW_LOOP_THRESHOLD_MS)
  {
    DPRINTF("SLOW LOOP: %lums (motor likely starved ~%lu steps)\n",
            loopDuration, (loopDuration * (unsigned long)SPEED_MAX) / 1000UL);
  }
  if (loopEnd - lastHeartbeat >= HEARTBEAT_MS)
  {
    DPRINTF("[uptime %lus] heap=%uK rssi=%ddBm mode=%s pos=%d%% target=%d%% moving=%d dist=%ld\n",
            loopEnd / 1000UL,
            (unsigned)(ESP.getFreeHeap() / 1024),
            (int)WiFi.RSSI(),
            state.currentMode == CALIBRATE ? "CAL" : "NORM",
            getCurrentPosition(),
            state.targetPercent,
            stepper.distanceToGo() != 0 ? 1 : 0,
            stepper.distanceToGo());
    lastHeartbeat = loopEnd;
  }

  yield();
}

void properLedDisplay()
{
  Buttons::blinkUpdate();
  // While confirmation blink runs, suppress slow blink to avoid overlap
  if (state.confirmBlinkActive)
    return;

  // LED indicator logic using LedControl
  // Blink LED if in calibration OR if not calibrated yet (initial setup/factory reset)
  bool shouldBlink = (state.currentMode == CALIBRATE) || (state.maxSteps == 0);
  if (shouldBlink)
  {
    // Slow blink during calibration/setup
    Led::blinkUpdate(LED_BLINK_INTERVAL_MS);
    return;
  }

  // Normal operation: bright when moving, dim when idle
  if (stepper.distanceToGo() != 0)
  {
    Led::setOn(255); // Full brightness while moving
  }
  else
  {
    Led::setBrightness(20); // Dim indicator light when idle
  }
}

void reset()
{
  helper.resetsettings();
  // Reset HomeSpan pairing: erase NVS and reboot
  nvs_flash_erase();
  delay(300);
  ESP.restart();
}

// Turn motor power off after inactivity (kept for state housekeeping)
void handleEngineControllerActivity()
{
  static bool wasMoving = false;
  static uint32_t stoppedAt = 0; // timestamp when motion stopped

  bool moving = (stepper.distanceToGo() != 0) || (state.currentMode == CALIBRATE);
  if (moving)
  {
    wasMoving = true;
    stoppedAt = 0; // reset any pending hold timer while moving
    return;
  }

  // Transition edge: moving -> stopped
  if (wasMoving)
  {
    wasMoving = false;
    stoppedAt = millis();
    state.holdingActive = false;
    if (state.currentMode != CALIBRATE)
    {
      if (!saveConfig())
      {
        DPRINTLN("Warning: Failed to save config after movement");
        state.lastMessage = F("Save config failed");
      }
      // Decide hold strategy
      if (HOLD_TORQUE_MS == 0)
      {
        // Immediate disable
        stepper.disableOutputs();
        stoppedAt = 0;
        state.holdingActive = false;
      }
      else if (HOLD_TORQUE_MS < 0)
      {
        // Infinite hold: ensure outputs are ON
        stepper.enableOutputs();
        state.holdingActive = true;
        // stoppedAt retained only for reference; never auto-disable
      }
      else
      {
        // Timed hold: keep outputs enabled until timeout
        stepper.enableOutputs();
        state.holdingActive = true;
      }
    }
    return; // wait for hold interval if configured
  }

  // If we are stopped and holding torque, check timeout (skip if infinite hold)
  if (state.holdingActive && stoppedAt != 0 && HOLD_TORQUE_MS > 0 && (millis() - stoppedAt) >= (uint32_t)HOLD_TORQUE_MS)
  {
    stepper.disableOutputs();
    stoppedAt = 0; // done holding
    state.holdingActive = false;
  }
}

// 0% = bottom (closed), 100% = top (open)
int getCurrentPosition()
{
  return calculatePercentForStep(state.currentStep);
}

bool loadConfig()
{
  if (!helper.loadconfig())
  {
    DPRINTLN("No config found, using defaults");
    return false;
  }

  JsonObjectConst json = helper.getconfig();
  state.currentStep = json["currentStep"] | 0;
  state.maxSteps = json["maxSteps"] | 0;
  state.targetPercent = json["targetPercent"] | 0;
  // Load raw calibration points if present
  state.upStep = json["rawUpStep"] | 0;
  state.downStep = json["rawDownStep"] | 0;

  DPRINTF("Loaded config: currentStep=%ld, maxSteps=%d, targetPercent=%d%%\n",
          state.currentStep, state.maxSteps, state.targetPercent);
  return true;
}

bool saveConfig()
{
  StaticJsonDocument<512> doc;
  doc["configVersion"] = 1;
  doc["currentStep"] = state.currentStep;
  doc["maxSteps"] = state.maxSteps;
  doc["targetPercent"] = state.targetPercent;
  doc["rawUpStep"] = state.upStep;
  doc["rawDownStep"] = state.downStep;
  unsigned long t0 = millis();
  bool ok = helper.saveconfig(doc);
  DPRINTF("saveConfig: %s step=%d max=%d target=%d%% upRaw=%d downRaw=%d (%lums)\n",
          ok ? "OK" : "FAIL",
          state.currentStep, state.maxSteps, state.targetPercent,
          state.upStep, state.downStep,
          millis() - t0);
  return ok;
}

void enableCalibrationMode()
{
  state.currentMode = CALIBRATE;
  state.currentCalibrationStep = INIT;
  // Only require release if a physical button is actually held now
  bool held = Buttons::isUpPressed() || Buttons::isDownPressed();
  state.calRequireRelease = held;
  // ensure no stale web jog causes motion on entry
  state.calJogDir = 0;
  stepper.moveTo(stepper.currentPosition());
  stepper.run();
  state.lastMovementTime = 0;
  // During calibration we want a smooth, continuous action via runSpeed()
  // Use a very large acceleration so run() won't divide-by-zero if called
  // accidentally; runSpeed() ignores the acceleration parameter entirely.
  stepper.setAcceleration(10000.0f);
  stepper.setMaxSpeed(CAL_SPEED);
  // clear any prior speed used by runSpeed()
  stepper.setSpeed(0.0f);
  DPRINTLN("Entered CALIBRATE mode (continuous runSpeed)");
}

void shadesControl()
{
  if (state.currentMode != NORMAL || state.maxSteps == 0)
    return;

  // Get target from state (updated by web/buttons or HomeKit will update via HomeSpan)
  // For now use a simple state variable

  long targetStep = (long)(100 - state.targetPercent) * state.maxSteps / 100;

  // Command stepper to the target (run() moves it)
  if (targetStep != stepper.targetPosition())
  {
    DPRINTF("Moving to target: %ld steps (%d%%)\n", targetStep, state.targetPercent);
    // Ensure coils are energized before a new move
    stepper.enableOutputs();
    stepper.moveTo(targetStep);
    state.lastMovementTime = millis();
    // User-facing feedback for Normal mode moves
    char msgBuf[24];
    snprintf(msgBuf, sizeof(msgBuf), "Moving to %d%%", state.targetPercent);
    state.lastMessage = msgBuf;
  }

  // If no distance left, we're stopped — set message only on transition
  static bool wasStopped = true;
  if (stepper.distanceToGo() == 0)
  {
    if (!wasStopped)
    {
      state.lastMessage = F("Stopped");
      wasStopped = true;
    }
    // Safety: ensure coils are de-energized when we report STOPPED
    if (HOLD_TORQUE_MS == 0)
    {
      stepper.disableOutputs();
    }
  }
  else
  {
    wasStopped = false;
  }
}
