#include "Globals.h"

#if ENABLE_MATTER
#include "matter_bridge.h"

extern int targetPercent;

#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
#include <Matter.h>
#include <MatterEndpoints/MatterDimmableLight.h>

namespace
{
  constexpr uint32_t REPORT_INTERVAL_MS = 600; // throttle updates while moving

  int clampPercent(int p)
  {
    if (p < 0)
      return 0;
    if (p > 100)
      return 100;
    return p;
  }

  uint8_t percentToLevel(int percent)
  {
    // Map 0-100% -> 0-255 with rounding
    long scaled = (long)percent * 255L + 50L;
    return (uint8_t)(scaled / 100L);
  }

  int levelToPercent(uint8_t level)
  {
    long scaled = (long)level * 100L + 127L;
    return (int)(scaled / 255L);
  }
}

namespace MatterBridge
{
  static MatterDimmableLight shadeEndpoint;
  static bool started = false;
  static int lastReportedPercent = -1;
  static uint32_t lastReportMs = 0;
  static bool ignoreNextCallback = false; // Block feedback from our own updates
  static bool lastOnState = false;        // Track last on/off state to detect real changes
  static uint32_t lastOffTime = 0;        // Track when we last went OFF to detect Matter state restoration
  static uint32_t lastOnTime = 0;         // Track when we last went ON to ignore immediate brightness changes

  static void publishPercent(int percent, bool force)
  {
    if (!started)
      return;
    int p = clampPercent(percent);
    if (!force && p == lastReportedPercent)
      return;

    uint8_t level = percentToLevel(p);
    bool on = (p > 0);

    // Update on/off first (reduces Matter errors during transitions)
    bool onChanged = (on != lastOnState);
    if (onChanged || force)
    {
      ignoreNextCallback = true; // Block callback for this update
      shadeEndpoint.setOnOff(on);
    }

    // Update brightness
    ignoreNextCallback = true; // Block callback for this update
    shadeEndpoint.setBrightness(level);

    lastReportedPercent = p;
    lastOnState = on; // Track what we published
  }

  static bool onMatterChange(bool on, uint8_t brightness)
  {
    // Ignore feedback from our own publishPercent updates
    if (ignoreNextCallback)
    {
      ignoreNextCallback = false;
      return true; // Silent ignore - these are our own updates
    }

    bool onChanged = (on != lastOnState);
    uint32_t now = millis();

    // Process all real Matter commands (feedback already filtered above)
    int percent;

    if (!on)
    {
      // OFF command → close fully
      percent = 0;
      lastOffTime = now;
      Serial.printf("Matter: OFF → 0%%\n");
    }
    else if (onChanged)
    {
      // ON after OFF → open fully (ignore brightness during transition)
      percent = 100;
      lastOnTime = now;
      Serial.printf("Matter: ON (was OFF) → 100%%\n");
    }
    else if (brightness == 1)
    {
      // Brightness=1 artifact while already ON → ignore
      return true;
    }
    else if (brightness < 20 && (now - lastOnTime) < 200)
    {
      // Ignore low brightness spike right after ON (< 20 within 200ms)
      Serial.printf("Matter: ignoring low brightness spike after ON (brightness=%d)\n", brightness);
      return true;
    }
    else
    {
      // Brightness change while ON → map to percent
      percent = levelToPercent(brightness);
      Serial.printf("Matter: brightness=%d → %d%%\n", brightness, percent);
    }

    int newTarget = clampPercent(percent);
    lastOnState = on; // Update tracked state

    Serial.printf("Matter: → %d%% (was %d%%)\n", newTarget, targetPercent);
    targetPercent = newTarget;
    return true;
  }

  void begin(int initialPercent)
  {
    int p = clampPercent(initialPercent);
    uint8_t level = percentToLevel(p);

    if (!shadeEndpoint.begin(p > 0, level))
    {
      DPRINTLN("Matter: failed to init endpoint");
      return;
    }

    shadeEndpoint.onChange(onMatterChange);
    Matter.begin();
    started = true;

    if (Matter.isDeviceCommissioned())
    {
      Serial.println("Matter: устройство уже добавлено");
      shadeEndpoint.updateAccessory();
    }
    else
    {
      Serial.println("\n=== MATTER PAIRING ===");
      Serial.println("QR Code URL: " + Matter.getOnboardingQRCodeUrl());
      Serial.println("Manual code: " + Matter.getManualPairingCode());
      Serial.println("======================\n");
    }

    publishPercent(p, true);
    lastReportMs = millis();
    DPRINTLN("Matter: ready");
  }

  void loop(int currentPercent, bool moving)
  {
    if (!started)
      return;

    uint32_t now = millis();
    bool percentChanged = (currentPercent != lastReportedPercent);

    // Throttle while moving to reduce attribute spam
    if (percentChanged && (!moving || (now - lastReportMs) >= REPORT_INTERVAL_MS))
    {
      publishPercent(currentPercent, false);
      lastReportMs = now;
    }
  }

  void factoryReset()
  {
    if (started)
    {
      Matter.decommission();
    }
  }

  bool isEnabled() { return started; }

  String getQRCodeUrl()
  {
    if (!started || Matter.isDeviceCommissioned())
      return "";
    return Matter.getOnboardingQRCodeUrl();
  }

  // Extract raw QR payload (string after data= in the URL), URL-decoded
  String getQRCodePayload()
  {
    String url = getQRCodeUrl();
    int idx = url.indexOf("data=");
    if (idx < 0)
      return "";
    String encoded = url.substring(idx + 5);

    // URL-decode minimal set (%XX)
    String decoded;
    for (int i = 0; i < encoded.length(); ++i)
    {
      if (encoded[i] == '%' && i + 2 < encoded.length())
      {
        char h1 = encoded[i + 1];
        char h2 = encoded[i + 2];
        auto hexVal = [](char c) -> int
        {
          if (c >= '0' && c <= '9')
            return c - '0';
          if (c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
          if (c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
          return 0;
        };
        int val = (hexVal(h1) << 4) | hexVal(h2);
        decoded += (char)val;
        i += 2;
      }
      else
      {
        decoded += encoded[i];
      }
    }
    return decoded;
  }

  String getPairingCode()
  {
    if (!started || Matter.isDeviceCommissioned())
      return "";
    return Matter.getManualPairingCode();
  }
}
#else
namespace MatterBridge
{
  void begin(int) {}
  void loop(int, bool) {}
  void factoryReset() {}
  bool isEnabled() { return false; }
  String getQRCodeUrl() { return ""; }
  String getPairingCode() { return ""; }
}
#endif
#else // ENABLE_MATTER
#include "matter_bridge.h"
namespace MatterBridge
{
  void begin(int) {}
  void loop(int, bool) {}
  void factoryReset() {}
  bool isEnabled() { return false; }
}
#endif
