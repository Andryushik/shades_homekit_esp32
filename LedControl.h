#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>

namespace Led
{
  void begin();
  void off();
  void setOn(uint8_t brightness = 255);
  void setBrightness(uint8_t brightness);
  void toggle();
  bool isOn();
  void blinkUpdate(uint32_t intervalMs); // Call in loop to handle automatic blinking
}

#endif
