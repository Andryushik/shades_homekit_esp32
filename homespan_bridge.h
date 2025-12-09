#ifndef HOMESPAN_BRIDGE_H
#define HOMESPAN_BRIDGE_H

namespace HomeSpanBridge
{
  void begin(int initialPercent);
  void loop();
  void updatePosition(int currentPercent);
  void factoryReset();
  bool isEnabled();
}

#endif
