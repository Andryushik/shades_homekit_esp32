#include "HomeSpanConfig.h"
#include "HomeSpan.h"
#include "HomeKitShade.h"

void homeSpanSetup()
{
  // Enable OTA updates (must be called before begin)
  homeSpan.enableOTA("28142814");

  // Initialize HomeSpan
  homeSpan.begin(Category::WindowCoverings, "Roller Shades");

  // Enable Auto-AP mode without password for easy WiFi configuration
  homeSpan.setApSSID("RollerShades-Setup");
  homeSpan.setApPassword("");
  homeSpan.enableAutoStartAP();

  homeSpan.setPairingCode("28142814");

  // Give HomeSpan internal tasks time to initialize
  delay(100);

  // Create HomeKit accessory structure
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Roller Shades");
  new Characteristic::Manufacturer("DIY");
  new Characteristic::Model("ESP32C6-Shades");
  new Characteristic::SerialNumber("RS-001");
  new RollerShade();
}
