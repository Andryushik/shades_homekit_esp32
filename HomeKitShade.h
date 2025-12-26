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
struct Blinds : Service::WindowCovering
{
  SpanCharacteristic *currentPosition;
  SpanCharacteristic *targetPosition;
  SpanCharacteristic *positionState;

  Blinds() : Service::WindowCovering()
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

    // Sync HomeKit with actual position (only update if changed)
    int currentPercent = getCurrentPosition();
    if (currentPosition->getVal() != currentPercent)
    {
      currentPosition->setVal(currentPercent);
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
