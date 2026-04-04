#include "Arduino.h"
#include "Helper.h"
#include "Globals.h"

// Define static storage for persistent config document
StaticJsonDocument<4096> Helper::_doc;

Helper::Helper()
{
  this->_configfile = "/config.json";
}

bool Helper::begin()
{
  if (LittleFS.begin(true, "/spiffs", 5, nullptr))
  {
    DPRINTLN("LittleFS OK");
    return true;
  }
  DPRINTLN("LittleFS failed");
  return false;
}

boolean Helper::loadconfig()
{
  File configFile = LittleFS.open(this->_configfile, "r");
  if (!configFile)
  {
    DPRINTLN(F("Failed to open config file"));
    return false;
  }

  size_t size = configFile.size();
  if (size > 4096)
  {
    DPRINTLN(F("Config file size is too large"));
    configFile.close();
    return false;
  }

  // Clear previous content and parse JSON into persistent document
  _doc.clear();
  DeserializationError err = deserializeJson(_doc, configFile);

  // Avoid leaving opened files
  configFile.close();

  if (err)
  {
    DPRINTLN("Failed to parse config file");
    return false;
  }
  // Ensure a config version exists
  if (!_doc["configVersion"].is<int>())
  {
    _doc["configVersion"] = 1;
  }
  return true;
}

JsonObjectConst Helper::getconfig() const
{
  return _doc.as<JsonObjectConst>();
}

boolean Helper::saveconfig(const JsonDocument &json)
{
  File configFile = LittleFS.open(this->_configfile, "w");
  if (!configFile)
  {
    DPRINTLN("Failed to open config file for writing");
    return false;
  }

  if (serializeJson(json, configFile) == 0)
  {
    DPRINTLN("Failed to write JSON to config file");
    configFile.close();
    return false;
  }
  configFile.flush();
  configFile.close();
  DPRINTLN("Saved JSON to LittleFS");
  return true;
}

void Helper::resetsettings()
{
  LittleFS.format();
}
