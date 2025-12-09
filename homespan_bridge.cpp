#include "Globals.h"
#include "homespan_bridge.h"
#include <HomeSpan.h>
#include <nvs_flash.h>

extern int targetPercent;

namespace
{
  // Custom WindowCovering service for roller shades
  struct RollerShade : Service::WindowCovering
  {
    SpanCharacteristic *currentPosition;
    SpanCharacteristic *targetPosition;
    SpanCharacteristic *positionState;

    RollerShade() : Service::WindowCovering()
    {
      currentPosition = new Characteristic::CurrentPosition(0); // 0-100
      targetPosition = new Characteristic::TargetPosition(0);   // 0-100
      positionState = new Characteristic::PositionState(2);     // 0=going down, 1=going up, 2=stopped

      Serial.println("HomeSpan: WindowCovering service created");
    }

    boolean update() override
    {
      // Called when HomeKit sends a new target position
      int newTarget = targetPosition->getNewVal();

      Serial.printf("HomeSpan: Target position changed to %d%%\n", newTarget);

      // Update global target
      targetPercent = newTarget;

      return true; // Return true to indicate success
    }

    void loop() override
    {
      // Called frequently - handle any periodic updates here if needed
    }
  };

  static RollerShade *shade = nullptr;
}

namespace HomeSpanBridge
{
  void begin(int initialPercent)
  {
    Serial.println("\n=== HomeSpan Initialization ===");

    // Initialize HomeSpan
    homeSpan.setLogLevel(1); // 0=off, 1=normal, 2=verbose
    homeSpan.begin(Category::WindowCoverings, "Roller Shades");

    // Create accessory
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("Roller Shades");
    new Characteristic::Manufacturer("DIY");
    new Characteristic::Model("ESP32C6-Shades");
    new Characteristic::SerialNumber("RS-001");

    // Add WindowCovering service
    shade = new RollerShade();

    // Set initial position
    if (shade)
    {
      shade->currentPosition->setVal(initialPercent);
      shade->targetPosition->setVal(initialPercent);
      shade->positionState->setVal(2); // Stopped
      Serial.printf("HomeSpan: Initial position set to %d%%\n", initialPercent);
    }

    Serial.println("=== HomeSpan Ready ===");
    Serial.println("Setup Code: 466-37-726");
    Serial.println("Open Home app and add accessory");
    Serial.println("============================\n");
  }

  void loop()
  {
    homeSpan.poll();
  }

  void updatePosition(int currentPercent)
  {
    if (!shade)
      return;

    // Update current position in HomeKit
    shade->currentPosition->setVal(currentPercent);

    // Update position state based on movement
    if (currentPercent == targetPercent)
    {
      shade->positionState->setVal(2); // Stopped
    }
    else if (currentPercent < targetPercent)
    {
      shade->positionState->setVal(1); // Going up (opening)
    }
    else
    {
      shade->positionState->setVal(0); // Going down (closing)
    }
  }

  void factoryReset()
  {
    // HomeSpan factory reset - erase NVS and restart
    Serial.println("HomeSpan: Factory reset - erasing pairing data");
    nvs_flash_erase();
    nvs_flash_init();
    ESP.restart();
  }

  bool isEnabled()
  {
    return true;
  }
}
