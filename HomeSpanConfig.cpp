#include "HomeSpanConfig.h"
#include "HomeSpan.h"
#include "HomeKitShade.h"

void homeSpanSetup()
{
  // Initialize HomeSpan
  homeSpan.begin(Category::WindowCoverings, "Blinds");

  // Enable Auto-AP mode without password for easy WiFi configuration
  homeSpan.setApSSID("Blinds-Setup");
  homeSpan.setApPassword("");
  homeSpan.enableAutoStartAP();

  homeSpan.setPairingCode("28142814");

  // Give HomeSpan internal tasks time to initialize
  delay(100);

  // Create HomeKit accessory structure
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name("Blinds");
  new Characteristic::Manufacturer("DIY");
  new Characteristic::Model("ESP32C6-Blinds");
  new Characteristic::SerialNumber("BL-001");
  new Blinds();
}
