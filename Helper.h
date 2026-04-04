#ifndef Helper_h
#define Helper_h

#include <Arduino.h>

#ifdef NO_INLINE
#undef NO_INLINE
#endif
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>

class Helper
{
public:
  Helper();
  bool begin(); // mount FS (format on failure)
  boolean loadconfig();
  JsonObjectConst getconfig() const;
  boolean saveconfig(const JsonDocument &json);
  void resetsettings();

private:
  static StaticJsonDocument<4096> _doc;
  String _configfile;
};

#endif
