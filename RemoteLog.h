#ifndef REMOTE_LOG_H
#define REMOTE_LOG_H

#include <Arduino.h>
#include <WiFi.h>

// Telnet debug console — streams all DPRINT/DPRINTLN/DPRINTF output
// to both Serial AND a single connected telnet client (port 23).
// Non-blocking: drops bytes when TCP buffer is full rather than stalling.
class RemoteLog : public Print
{
public:
  void begin();
  void loop(); // call from main loop — accepts clients, drains input
  size_t write(uint8_t c) override;
  size_t write(const uint8_t *buffer, size_t size) override;
  bool hasClient();

private:
  WiFiServer _server{23};
  WiFiClient _client;
  bool _clientActive = false; // set true when client accepted, false when we stop()
};

extern RemoteLog rlog;

#endif
