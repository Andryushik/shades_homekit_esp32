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
  return LittleFS.begin(true, "/spiffs", 5, "spiffs");
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

  _doc.clear();
  DeserializationError err = deserializeJson(_doc, configFile);
  configFile.close();

  if (err)
  {
    DPRINTLN("Failed to parse config file");
    return false;
  }
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
    return false;

  // Copy to local document — ArduinoJson v6 cannot serialize
  // correctly through a polymorphic const JsonDocument& reference
  StaticJsonDocument<1024> doc;
  JsonObject dst = doc.to<JsonObject>();
  for (JsonPairConst kv : json.as<JsonObjectConst>())
    dst[kv.key()] = kv.value();

  size_t written = serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();
  return written > 0;
}

void Helper::resetsettings()
{
  LittleFS.format();
}
