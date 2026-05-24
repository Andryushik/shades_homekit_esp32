#ifndef HOMEKIT_SHADE_H
#define HOMEKIT_SHADE_H

#include "HomeSpan.h"
#include "Globals.h"
#include <AccelStepper.h>

// External references from main
extern ShadesState state;
extern AccelStepper stepper;
extern int getCurrentPosition();

// HomeSpan custom WindowCovering service for blinds
struct RollerShade : Service::WindowCovering
{
  SpanCharacteristic *currentPosition;
  SpanCharacteristic *targetPosition;
  SpanCharacteristic *positionState;

  RollerShade() : Service::WindowCovering()
  {
    // Initialize with saved position from state
    int currentPos = getCurrentPosition();
    currentPosition = new Characteristic::CurrentPosition(currentPos);
    targetPosition = new Characteristic::TargetPosition(state.targetPercent);
    positionState = new Characteristic::PositionState(2); // 2=stopped
  }

  boolean update() override
  {
    // Called when HomeKit sends a new target position
    int newTarget = targetPosition->getNewVal();
    DPRINTF("HK update: target %d%% -> %d%% (curPos=%d%%)\n",
            state.targetPercent, newTarget, getCurrentPosition());
    state.targetPercent = newTarget;
    return true;
  }

  void loop() override
  {
    // Safety check: only update if maxSteps is initialized
    if (state.maxSteps == 0)
      return;

    // Sync targetPosition with state (in case changed from web UI)
    if (targetPosition->getVal() != state.targetPercent)
    {
      targetPosition->setVal(state.targetPercent);
    }

    // Sync HomeKit with actual position — throttled to reduce TLS overhead
    // on single-core ESP32-C6. Always report final position on stop.
    int currentPercent = getCurrentPosition();
    bool moving = (stepper.distanceToGo() != 0);

    static unsigned long lastPosUpdate = 0;
    unsigned long now = millis();
    bool shouldUpdate = !moving                         // always report final position
                        || (now - lastPosUpdate >= 500); // throttle during motion

    if (shouldUpdate && currentPosition->getVal() != currentPercent)
    {
      DPRINTF("HK notify: currentPos %d%% -> %d%% (moving=%d)\n",
              (int)currentPosition->getVal(), currentPercent, moving ? 1 : 0);
      currentPosition->setVal(currentPercent);
      lastPosUpdate = now;
    }

    // Update position state based on movement (only update if changed)
    int newState;
    if (stepper.distanceToGo() == 0)
    {
      newState = 2; // stopped
    }
    else if (currentPercent < state.targetPercent)
    {
      newState = 1; // opening (going up)
    }
    else
    {
      newState = 0; // closing (going down)
    }

    if (positionState->getVal() != newState)
    {
      positionState->setVal(newState);
    }
  }
};

#endif
