#include "ButtonActions.h"
#include <Arduino.h>
#include <AccelStepper.h>
#include "Globals.h"
#include "pins.h"
#include "LedControl.h"

// Access acceleration constant from main TU
extern const float ACCEL;

// Calculate percentage for any step position (inverse of shadesControl calculation)
int calculatePercentForStep(long stepPosition)
{
  if (state.maxSteps <= 0)
    return 0;

  // Same formula as getCurrentPosition() but for arbitrary step position
  long numer = 100L * ((long)state.maxSteps - stepPosition);
  int percent = (int)((numer + (state.maxSteps / 2)) / (long)state.maxSteps);

  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  return percent;
}

extern AccelStepper stepper;
extern ShadesState state;
extern int getCurrentPosition();
extern bool saveConfig();
extern void reset();

// Blink state (used for confirmation pulses)
static int ba_blinkStepsRemaining = 0;
static unsigned long ba_blinkIntervalMs = 0;
static unsigned long ba_blinkLastToggleMs = 0;
static bool ba_blinkLedState = false;

void BA_startBlink(int times, int ms)
{
  if (times <= 0 || ms == 0)
    return;
  ba_blinkStepsRemaining = times * 2;
  ba_blinkIntervalMs = ms;
  ba_blinkLastToggleMs = millis();
  ba_blinkLedState = true;
  Led::setOn();
  ba_blinkStepsRemaining--;
}

void BA_blinkUpdate()
{
  if (ba_blinkStepsRemaining <= 0)
    return;
  unsigned long now = millis();
  if ((now - ba_blinkLastToggleMs) >= ba_blinkIntervalMs)
  {
    ba_blinkLedState = !ba_blinkLedState;
    if (ba_blinkLedState)
      Led::setOn();
    else
      Led::off();
    ba_blinkLastToggleMs = now;
    ba_blinkStepsRemaining--;
    if (ba_blinkStepsRemaining == 0)
    {
      Led::off();
      ba_blinkLedState = false;
      if (state.confirmBlinkActive)
      {
        if (state.exitCalibrationAfterBlink)
        {
          state.currentMode = NORMAL;
          state.exitCalibrationAfterBlink = false;
        }
        state.confirmBlinkActive = false;
      }
    }
  }
}

void BA_moveToPercent(int percent)
{
  int tp = percent;
  if (tp < 0)
    tp = 0;
  else if (tp > 100)
    tp = 100;
  state.targetPercent = tp;
  DPRINTF("Target set to %d%%\n", tp);
  if (tp == 100)
    state.lastMessage = F("Moving UP");
  else if (tp == 0)
    state.lastMessage = F("Moving DOWN");
  else
  {
    char buf[24];
    snprintf(buf, sizeof(buf), "Moving to %d%%", tp);
    state.lastMessage = buf;
  }
}

void BA_stopMotion()
{
  // SMOOTH STOP with deceleration - calculate stop position and let motor decelerate naturally
  long currentPos = stepper.currentPosition();
  float currentSpeed = stepper.speed();
  float absSpeed = abs(currentSpeed);
  long targetPosBefore = stepper.targetPosition();
  DPRINTF("BA_stopMotion: curPos=%ld speed=%.1f targetBefore=%ld targetPercentBefore=%d%%\n",
          currentPos, currentSpeed, targetPosBefore, state.targetPercent);

  if (absSpeed > 0.1f) // Only if moving
  {
    // Calculate deceleration distance: v² = 2*a*s → s = v²/(2*a)
    float decelDistance = (absSpeed * absSpeed) / (2.0f * ACCEL);

    // Set new target to stop with smooth deceleration
    long stopPos = currentPos + (long)(currentSpeed > 0 ? decelDistance : -decelDistance);
    stepper.moveTo(stopPos);

    // Immediately update local target to STOP position to prevent overwrite
    int stopPercent = calculatePercentForStep(stopPos);
    state.targetPercent = stopPercent;

    state.lastMessage = F("Stopping...");
  }
  else
  {
    // Already stopped or very slow - instant stop
    stepper.setCurrentPosition(currentPos);
    stepper.moveTo(currentPos);

    // Update local target immediately
    int currentPercent = getCurrentPosition();
    state.targetPercent = currentPercent;
    state.lastMessage = F("Stopped");
  }
}

void BA_calToggleJog(int8_t dir)
{
  if (state.calJogDir == dir)
  {
    DPRINTLN("BA: Calibration jog stop");
    state.calJogDir = 0;
    state.lastMessage = F("Stopped");
  }
  else
  {
    DPRINT("BA: Calibration jog start dir=");
    DPRINTLN(dir);
    state.calJogDir = dir;
    state.lastMessage = (dir < 0) ? F("Moving UP") : F("Moving DOWN");
  }
}

void BA_calStop()
{
  state.calJogDir = 0;
  state.lastMessage = F("Stopped");
}

void BA_exitCalibrationNoSave()
{
  state.calJogDir = 0;
  if (state.currentMode == CALIBRATE)
  {
    DPRINTLN("BA: Exit calibration (no save)");
    state.currentMode = NORMAL;
  }
}

void BA_calibrationSaveTop()
{
  state.calJogDir = 0;
  state.upStep = stepper.currentPosition();
  state.currentCalibrationStep = UP_KNOWN;
  DPRINT("Calibration: saved TOP raw position = ");
  DPRINTLN(state.upStep);
  DPRINTLN("Calibration: MAIN short press (save TOP)");
  char topBuf[40];
  snprintf(topBuf, sizeof(topBuf), "Saved top position (step %d)", state.upStep);
  state.lastMessage = topBuf;
  state.confirmBlinkActive = true;
  state.exitCalibrationAfterBlink = false;
  BA_startBlink(5, 80);
}

bool BA_calibrationSaveBottom()
{
  state.calJogDir = 0;
  state.downStep = stepper.currentPosition();
  DPRINT("Calibration: saved BOTTOM raw position = ");
  DPRINTLN(state.downStep);
  DPRINTLN("Calibration: MAIN short press (save BOTTOM)");
  int travel = abs(state.downStep - state.upStep);
  DPRINT("Calibration: measured travel = ");
  DPRINTLN(travel);
  if (travel < MIN_TRAVEL)
  {
    DPRINTLN("Calibration: travel too small, aborting save");
    char travelBuf[48];
    snprintf(travelBuf, sizeof(travelBuf), "Travel too small: %d < %d", travel, MIN_TRAVEL);
    state.lastMessage = travelBuf;
    return false;
  }
  state.maxSteps = travel;
  int rebasedCurrent = state.currentStep - state.upStep;
  stepper.setCurrentPosition(rebasedCurrent);
  state.currentStep = rebasedCurrent;
  state.targetPercent = 0;
  state.confirmBlinkActive = true;
  state.exitCalibrationAfterBlink = true;
  stepper.setMaxSpeed(SPEED_MAX);
  stepper.setAcceleration(ACCEL);
  if (!saveConfig())
  {
    DPRINTLN("Warning: Failed to save config during calibration");
    state.lastMessage = F("Calibration save failed");
    return false;
  }
  DPRINTLN("Calibration: finished, rebased and saved");
  char calBuf[48];
  snprintf(calBuf, sizeof(calBuf), "Calibration saved: travel %d steps", travel);
  state.lastMessage = calBuf;
  BA_startBlink(5, 80);
  return true;
}

void BA_factoryReset()
{
  DPRINTLN("BA: Factory reset requested");
  reset();
}
