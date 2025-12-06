// Seeed Studio XIAO ESP32C6 pin assignments (safe defaults per Seeed docs)
// Reference: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/
// Notes:
// - Buttons use INPUT_PULLUP, pressed == LOW.
// - ULN2003 driver for 28BYJ-48 stepper via AccelStepper HALF4WIRE.
// - LED kept dim at standby via PWM (LEDC).

// On-board LED (single LED). Prefer Arduino's LED_BUILTIN if defined.
#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#else
#define LED_PIN 15 // fallback; adjust if your board differs
#endif

// Buttons
#define BUTTON_UP_PIN 8   // XIAO D8 (GPIO8)
#define BUTTON_DOWN_PIN 9 // XIAO D9 (GPIO9)

// Motor ULN2003 driver coil pins (AccelStepper 4-wire HALF4WIRE)
// Wiring on XIAO header: choose sequential pins for cleaner routing
#define MOTOR_IN1 1 // D1 (GPIO1)
#define MOTOR_IN2 2 // D2 (GPIO2)
#define MOTOR_IN3 3 // D3 (GPIO3)
#define MOTOR_IN4 4 // D4 (GPIO4)
