#include "HomeSpanConfig.h"
#include "HomeSpan.h"
#include "HomeKitShade.h"

void homeSpanSetup()
{
  // Initialize HomeSpan
  Serial.println("DEBUG: About to call homeSpan.begin()...");
  homeSpan.begin(Category::WindowCoverings, "Roller Shades");
  Serial.println("DEBUG: homeSpan.begin() completed!");

  // Set HomeSpan to use default port 80 (HAP server)
  // Web UI will use port 8080
  Serial.println("DEBUG: HomeSpan HAP server using default port 80");

  // Enable Auto-AP mode without password for easy WiFi configuration
  homeSpan.setApSSID("RollerShades-Setup"); // Custom AP name
  homeSpan.setApPassword("");               // Empty password = open network
  homeSpan.enableAutoStartAP();
  Serial.println("DEBUG: Auto-AP mode enabled (open network)!");

  homeSpan.setPairingCode("28142814");
  Serial.println("DEBUG: Pairing code set to 28142814");

  // Give HomeSpan internal tasks time to initialize
  delay(100);
  Serial.println("DEBUG: Creating SpanAccessory...");

  // Create HomeKit accessory structure
  new SpanAccessory();
  Serial.println("DEBUG: SpanAccessory created!");

  new Service::AccessoryInformation();
  Serial.println("DEBUG: AccessoryInformation created!");

  new Characteristic::Identify();
  new Characteristic::Name("Roller Shades");
  new Characteristic::Manufacturer("DIY");
  new Characteristic::Model("ESP32C6-Shades");
  new Characteristic::SerialNumber("RS-001");
  Serial.println("DEBUG: Characteristics created!");

  new RollerShade(); // Custom WindowCovering service with update/loop
  Serial.println("DEBUG: RollerShade service created!");
}
