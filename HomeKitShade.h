#ifndef HOMEKIT_SHADE_H
#define HOMEKIT_SHADE_H

#include "HomeSpan.h"
#include "Globals.h"
#include <AccelStepper.h>

// External references from main
extern ShadesState state;
extern AccelStepper stepper;
extern int getCurrentPosition();

// HomeSpan custom WindowCovering service for roller shades
struct RollerShade : Service::WindowCovering
{
  SpanCharacteristic *currentPosition;
  SpanCharacteristic *targetPosition;
  SpanCharacteristic *positionState;

  RollerShade() : Service::WindowCovering()
  {
    currentPosition = new Characteristic::CurrentPosition(0);
    targetPosition = new Characteristic::TargetPosition(0);
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

    // Sync HomeKit with actual position
    int currentPercent = getCurrentPosition();
    currentPosition->setVal(currentPercent);

    // Update position state based on movement
    if (stepper.distanceToGo() == 0)
    {
      positionState->setVal(2); // stopped
    }
    else if (currentPercent < state.targetPercent)
    {
      positionState->setVal(1); // opening (going up)
    }
    else
    {
      positionState->setVal(0); // closing (going down)
    }
  }
};

#endif
