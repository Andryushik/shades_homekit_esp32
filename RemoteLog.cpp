#include "RemoteLog.h"
#include "Diagnostics.h" // for gResetReason
#include <lwip/sockets.h>
#include <netinet/tcp.h>

RemoteLog rlog;

// Global debug output — referenced by DPRINT/DPRINTLN/DPRINTF macros in Globals.h
Print &debugOut = rlog;

void RemoteLog::begin()
{
  _server.begin();
  _server.setNoDelay(true);
}

void RemoteLog::loop()
{
  // Accept new client (one at a time)
  if (_server.hasClient())
  {
    WiFiClient incoming = _server.accept();
    // Always take the new client (replacing any prior). We can't reliably tell
    // if the prior is alive without false-positive disconnects, and one user
    // with one logger is the realistic case anyway.
    if (_clientActive)
      _client.stop();
    _client = incoming;
    _clientActive = true;
    _client.setNoDelay(true);

    // TCP keepalive: detect dead peers without app-level heartbeat.
    // idle=10 s / interval=5 s / count=3 → dead peer detected ~25 s after.
    int fd = _client.fd();
    if (fd >= 0)
    {
      int yes = 1, idle = 10, intvl = 5, cnt = 3;
      setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
      setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
      setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
      setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    }

    _client.printf("=== Blinds Debug Console (%s) build %s %s reset=%s uptime=%lus ===\r\n",
                    WiFi.localIP().toString().c_str(),
                    __DATE__, __TIME__,
                    gResetReason,
                    millis() / 1000UL);
  }

  // Drain any input from the client. DO NOT call _client.connected() here —
  // its peek-recv returns false on a freshly accepted idle socket and would
  // make us close the live session immediately. Dead clients are detected by
  // TCP keepalive (configured above on accept) and by a fresh accept() which
  // replaces _client. Cheap and reliable.
  if (_clientActive && _client.available())
  {
    while (_client.available())
      _client.read();
  }
}

bool RemoteLog::hasClient()
{
  return _clientActive;
}

// NOTE: do NOT call _client.connected() or _client.availableForWrite() here.
// On ESP32 Arduino 3.x, connected() does a peek-recv that can transiently
// report false on an idle established socket, and availableForWrite() can
// return 0 right after connect. Both led to DPRINT bytes being silently
// dropped. We just attempt the write — if the socket is bad, write() returns
// 0 and we discard normally. Disconnect detection happens in loop() instead.
// Guard with our cached flag, NOT WiFiClient::operator bool or .connected()
// — both call into LWIP's peek-recv which has been observed to drop bytes
// silently right after accept on ESP32 Arduino 3.x.
size_t RemoteLog::write(uint8_t c)
{
  Serial.write(c);
  if (_clientActive)
    _client.write(c);
  return 1;
}

size_t RemoteLog::write(const uint8_t *buffer, size_t size)
{
  Serial.write(buffer, size);
  if (_clientActive)
    _client.write(buffer, size);
  return size;
}
