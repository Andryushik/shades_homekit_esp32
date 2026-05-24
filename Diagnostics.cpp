#include "Diagnostics.h"
#include "Globals.h"
#include <WiFi.h>
#include "esp_system.h"

// ESP-IDF private API to disable the brownout detector at runtime.
// Implemented inside IDF (esp_brownout_detector.c) — symbol is linkable from
// the arduino-esp32 core because the IDF is built into the same binary.
extern "C" void esp_brownout_disable(void);

const char *gResetReason = "UNKNOWN";

static const char *resetReasonStr()
{
  switch (esp_reset_reason())
  {
  case ESP_RST_POWERON:
    return "POWERON";
  case ESP_RST_EXT:
    return "EXTERNAL";
  case ESP_RST_SW:
    return "SOFTWARE";
  case ESP_RST_PANIC:
    return "PANIC";
  case ESP_RST_INT_WDT:
    return "INT_WDT";
  case ESP_RST_TASK_WDT:
    return "TASK_WDT";
  case ESP_RST_WDT:
    return "WDT";
  case ESP_RST_DEEPSLEEP:
    return "DEEPSLEEP";
  case ESP_RST_BROWNOUT:
    return "BROWNOUT";
  case ESP_RST_SDIO:
    return "SDIO";
  default:
    return "UNKNOWN";
  }
}

void diag_init()
{
  gResetReason = resetReasonStr();
  DPRINTF("Reset reason: %s\n", gResetReason);
}

void diag_installWiFiEventLogger()
{
  WiFi.onEvent([](arduino_event_t *event)
               {
    switch (event->event_id)
    {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DPRINTF("WiFi disconnected (reason=%d)\n",
              event->event_info.wifi_sta_disconnected.reason);
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DPRINTF("WiFi got IP=%s rssi=%ddBm\n",
              IPAddress(event->event_info.got_ip.ip_info.ip.addr).toString().c_str(),
              WiFi.RSSI());
      break;
    default:
      break;
    } });
}

void diag_disableBrownout()
{
  esp_brownout_disable();
  DPRINTLN("WARN: brownout detector DISABLED (temporary workaround)");
}
