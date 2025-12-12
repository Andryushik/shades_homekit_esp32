#include "LedControl.h"
#include "pins.h"
#include <Arduino.h>

#define LED_PWM_FREQ 5000
#define LED_PWM_RESOLUTION 8

namespace Led
{
  static uint8_t currentBrightness = 0;
  static uint32_t nextBlinkMillis = 0;

  void begin()
  {
    // Setup LEDC PWM for LED (new Arduino ESP32 API)
    ledcAttach(LED_PIN, LED_PWM_FREQ, LED_PWM_RESOLUTION);
    off();
  }

  void off()
  {
    ledcWrite(LED_PIN, 255); // Inverted: HIGH = OFF
    currentBrightness = 0;
  }

  void setOn(uint8_t brightness)
  {
    // If caller passes 0, treat it as full ON to keep "on" semantics
    uint8_t b = (brightness == 0) ? 255 : brightness;
    setBrightness(b);
  }

  void setBrightness(uint8_t brightness)
  {
    ledcWrite(LED_PIN, 255 - brightness); // Inverted: invert duty cycle
    currentBrightness = brightness;
  }

  void toggle()
  {
    if (currentBrightness > 0)
    {
      ledcWrite(LED_PIN, 255); // OFF (inverted)
      currentBrightness = 0;
    }
    else
    {
      ledcWrite(LED_PIN, 0); // ON at full brightness (inverted)
      currentBrightness = 255;
    }
  }

  bool isOn()
  {
    return currentBrightness > 0;
  }

  void blinkUpdate(uint32_t intervalMs)
  {
    const uint32_t now = millis();
    if (now >= nextBlinkMillis)
    {
      nextBlinkMillis = now + intervalMs;
      toggle();
    }
  }
}
