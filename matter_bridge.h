#ifndef MATTER_BRIDGE_H
#define MATTER_BRIDGE_H

#include <Arduino.h>

// Thin wrapper around the Arduino Matter library that maps the shades'
// target/position into a Matter endpoint. Uses a dimmable light endpoint
// (no native window covering endpoint is available in the current Arduino
// Matter package) where brightness == shades position (0-100%).
namespace MatterBridge
{
  // Initialize Matter endpoint/state using the current shades position.
  void begin(int initialPercent);

  // Periodically push position changes to Matter (call from loop()).
  void loop(int currentPercent, bool moving);

  // Decommission Matter state during factory reset.
  void factoryReset();

  // True when Matter endpoint was initialized.
  bool isEnabled();

  // Get Matter pairing info (empty if commissioned)
  String getQRCodeUrl();
  String getPairingCode();
  String getQRCodePayload();
}

#endif
