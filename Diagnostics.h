#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

// Diagnostic / power-rail helpers. All DPRINT output is gated by SHADES_DEBUG
// in Globals.h, so these are cheap in release builds (mostly no-ops).

// Set in diag_init() from esp_reset_reason(); read by RemoteLog's connect
// banner so a reconnecting telnet client sees the last reboot cause (the
// boot-time DPRINT is missed because no client is attached at boot).
extern const char *gResetReason;

// Capture the reset reason and print a boot banner with it.
// Call ONCE early in setup(), after Serial.begin().
void diag_init();

// Register a WiFi event handler that logs STA disconnect/connect via DPRINTF.
// Call ONCE in setup(), before WiFi connects.
void diag_installWiFiEventLogger();

// TEMPORARY brownout workaround — disable the BOD detector so the chip
// survives motor pulse VCC dips. Remove this call once a bulk capacitor is
// installed on the 5V rail next to the XIAO.
void diag_disableBrownout();

#endif
