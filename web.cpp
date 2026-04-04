#include "web.h"
#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "Globals.h"
#include "Buttons.h"
#include "ButtonActions.h"
#include <AccelStepper.h>
#include "HomeSpan.h"
#include "web_style.h"
#include "web_html.h"
#include "web_script.h"

// Accessors provided by main translation unit
extern int getCurrentPosition();
extern AccelStepper stepper;

// Externals from main/Buttons
extern ShadesState state;
extern void enableCalibrationMode();
extern bool saveConfig();
extern void reset();

static WebServer server(8080);

static void handleRoot()
{
  // Build HTML body with server-side placeholders replaced
  String body = FPSTR(WEB_HTML_BODY);
  String modeClass = (state.currentMode == CALIBRATE) ? "badge-calibrate" : "badge-normal";
  String modeText = (state.currentMode == CALIBRATE) ? "CALIBRATE" : "NORMAL";
  String posStr = String(getCurrentPosition());

  body.replace("MODE_CLASS", modeClass);
  body.replace("MODE_PLACEHOLDER", modeText);
  body.replace("POS_PLACEHOLDER", posStr);
  body.replace("CUR_PLACEHOLDER", String(state.currentStep));
  body.replace("MAX_PLACEHOLDER", String(state.maxSteps));

  bool hasPairings = (homeSpan.controllerListBegin() != homeSpan.controllerListEnd());
  body.replace("HOMEKIT_DISPLAY", hasPairings ? "display:none" : "display:block");

  // Send chunked: HEAD + CSS + BODY + JS + END
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=UTF-8", "");
  server.sendContent_P(WEB_HTML_HEAD);
  server.sendContent_P(WEB_CSS);
  server.sendContent(body);
  server.sendContent_P(WEB_JS);
  server.sendContent_P(WEB_HTML_END);
  server.sendContent("");
}

static void sendQuickResponse()
{
  server.send(200, "text/plain", "OK");
}

static void handleCalStart()
{
  if (state.currentMode != CALIBRATE)
  {
    DPRINTLN("WEB: Start Calibration requested");
    enableCalibrationMode();
  }
  sendQuickResponse();
}

static void handleCalStop()
{
  // stop any calibration jogging and return to NORMAL
  BA_exitCalibrationNoSave();
  DPRINTLN("WEB: Exit Calibration requested");
  sendQuickResponse();
}

static void handleUpStart()
{
  if (state.currentMode == CALIBRATE)
  {
    BA_calToggleJog(-1);
    DPRINTLN("WEB: Calibration jog UP toggle");
  }
  else
  {
    BA_moveToPercent(100);
    DPRINTLN("WEB: Move to 100% (UP)");
  }
  sendQuickResponse();
}

static void handleDownStart()
{
  if (state.currentMode == CALIBRATE)
  {
    BA_calToggleJog(1);
    DPRINTLN("WEB: Calibration jog DOWN toggle");
  }
  else
  {
    BA_moveToPercent(0);
    DPRINTLN("WEB: Move to 0% (DOWN)");
  }
  sendQuickResponse();
}

static void handlePreset(int percent)
{
  if (state.currentMode != CALIBRATE)
  {
    BA_moveToPercent(percent);
    DPRINTF("WEB: Move to %d%% preset\n", percent);
  }
  sendQuickResponse();
}

static void handleHoldStop()
{
  if (state.currentMode == CALIBRATE)
  {
    DPRINTLN("WEB: Calibration jog STOP");
    BA_calStop();
  }
  else
  {
    DPRINTLN("WEB: STOP command");
    BA_stopMotion();
  }
  sendQuickResponse();
}

// Use encapsulated calibration save routines from Buttons namespace

static void handleSaveTop()
{
  if (state.currentMode == CALIBRATE)
  {
    DPRINTLN("WEB: Save TOP position");
    Buttons::calibrationSaveTop();
  }
  sendQuickResponse();
}

static void handleSaveBottom()
{
  if (state.currentMode == CALIBRATE)
  {
    DPRINTLN("WEB: Save BOTTOM position");
    Buttons::calibrationSaveBottom();
  }
  sendQuickResponse();
}

static void handleReboot()
{
  DPRINTLN("WEB: Safe Reboot requested");
  int currentPercent = getCurrentPosition();
  state.targetPercent = currentPercent;
  stepper.stop();
  state.currentStep = stepper.currentPosition();
  if (!saveConfig())
  {
    DPRINTLN("Warning: Failed to save config during reboot");
    // Continue with reboot anyway, but log the error
  }

  String page;
  page.reserve(400);
  page += F("<!doctype html><html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Rebooting</title><style>body{font-family:sans-serif;margin:24px}a{color:#0645ad;text-decoration:none}a:hover{text-decoration:underline}</style></head><body>");
  page += F("<h2>Rebooting...\n</h2>");
  page += F("<p>Shades Controller is restarting now. Please wait.</p>");
  page += F("<script>setTimeout(function(){location.href='/'},15000);</script>");
  page += F("</body></html>");
  server.send(200, "text/html", page);
  delay(300);
  ESP.restart();
}

static void handleFactoryPost()
{
  DPRINTLN("WEB: Factory Reset requested");
  server.send(200, "text/plain", "Factory resetting...\n");
  delay(200);
  // Centralized factory reset
  BA_factoryReset();
}

static void handleStatus()
{
  // Small JSON document for status
  StaticJsonDocument<512> doc;
  doc["currentStep"] = state.currentStep;
  doc["maxSteps"] = state.maxSteps;
  doc["mode"] = (state.currentMode == CALIBRATE) ? "CALIBRATE" : "NORMAL";
  doc["position"] = getCurrentPosition();
  doc["msg"] = state.lastMessage;
  doc["moving"] = (stepper.distanceToGo() != 0);
  doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  String ipStr = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString()
                                                 : ((WiFi.getMode() & WIFI_AP) ? WiFi.softAPIP().toString() : "");
  doc["ip"] = ipStr;
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "application/json", out);
}

void webBegin()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/factory", HTTP_POST, handleFactoryPost);
  server.on("/cal/start", HTTP_POST, handleCalStart);
  server.on("/cal/stop", HTTP_POST, handleCalStop);
  server.on("/cal/up/start", HTTP_POST, handleUpStart);
  server.on("/cal/down/start", HTTP_POST, handleDownStart);
  server.on("/cal/hold/stop", HTTP_POST, handleHoldStop);
  server.on("/cal/saveTop", HTTP_POST, handleSaveTop);
  server.on("/cal/saveBottom", HTTP_POST, handleSaveBottom);
  server.on("/preset/30", HTTP_POST, []()
            { handlePreset(30); });
  server.on("/preset/50", HTTP_POST, []()
            { handlePreset(50); });
  server.on("/preset/70", HTTP_POST, []()
            { handlePreset(70); });
  server.on("/reboot", HTTP_POST, handleReboot);
  server.begin();
}

void webLoop()
{
  server.handleClient();
}
