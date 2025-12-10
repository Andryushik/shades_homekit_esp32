// Seeed Studio XIAO ESP32C6 pin assignments (official Arduino variant)
// Reference: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/ and
// variant pins_arduino.h:
// D0=GPIO0, D1=GPIO1, D2=GPIO2, D3=GPIO21, D4=GPIO22, D5=GPIO23, D6=GPIO16,
// D7=GPIO17, D8=GPIO19, D9=GPIO20, D10=GPIO18; on-board RGB pixel DIN=GPIO9.
// Notes:
// - Buttons use INPUT_PULLUP, pressed == LOW.
// - ULN2003 driver for 28BYJ-48 stepper via AccelStepper HALF4WIRE.
// - On-board RGB LED is WS2812-compatible on GPIO9 (not the D9 header pin).
#include <Arduino.h>

// Onboard LED (standard GPIO, not addressable RGB)
// ESP32C6 XIAO has LED on GPIO15
#define LED_PIN 15

// Buttons
#define BUTTON_UP_PIN 19   // XIAO D8 (GPIO19 / SPI SCK)
#define BUTTON_DOWN_PIN 20 // XIAO D9 (GPIO20 / SPI MISO)

// Motor ULN2003 driver coil pins (AccelStepper 4-wire HALF4WIRE)
// Wiring on XIAO header: choose sequential pins for cleaner routing
#define MOTOR_IN1 1  // D1 (GPIO1)
#define MOTOR_IN2 2  // D2 (GPIO2)
#define MOTOR_IN3 21 // D3 (GPIO21 / SPI SS)
#define MOTOR_IN4 22 // D4 (GPIO22 / I2C SDA)
