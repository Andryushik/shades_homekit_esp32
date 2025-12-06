#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "net_wifi.h"
#include "Globals.h"

void wifiConnect()
{
  WiFi.mode(WIFI_MODE_STA);
  WiFi.setHostname("roller_shades");
  WiFi.setSleep(false);
  DPRINTLN("WiFi connecting...");
  WiFi.begin();
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 20000)
  {
    delay(200);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    DPRINTLN("WiFi connect failed; starting AP 'RollerShades'");
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP("RollerShades");
  }
  DPRINTF("WiFi mode: %s, IP: %s\n", (WiFi.getMode() == WIFI_MODE_AP ? "AP" : "STA"), WiFi.localIP().toString().c_str());

  // Start mDNS when in STA
  if (WiFi.getMode() != WIFI_MODE_AP)
  {
    const char *host = "roller_shades";
    for (uint8_t i = 0; i < 5; ++i)
    {
      if (MDNS.begin(host))
      {
        MDNS.setInstanceName("Roller Shades");
        MDNS.addService("http", "tcp", 80);
        DPRINTLN("mDNS started: roller_shades.local");
        break;
      }
      delay(500);
    }
  }
}
