#pragma once
#include <Arduino.h>

// ── WiFiManager ───────────────────────────────────────────────────────────────
// Owns all Wi-Fi logic:
//   • Reads / writes credentials from NVS (Preferences)
//   • Attempts connection on boot if credentials exist
//   • Reconnects automatically every WIFI_RETRY_MS if disconnected
//   • Exposes a simple status API used by the serial CLI and web server
// ─────────────────────────────────────────────────────────────────────────────

namespace WiFiManager {

// Call once in setup(). Loads stored credentials and starts connection attempt.
void begin();

// Call every loop iteration — handles async reconnect timing.
void handle();

// Persist new credentials and immediately attempt connection.
// Returns true if the store succeeded (does NOT wait for association).
bool setCredentials(const String& ssid, const String& password);

// Wipe stored credentials and disconnect.
void resetCredentials();

// True when currently associated and IP assigned.
bool isConnected();

// Returns true exactly ONCE on the loop tick when WiFi first connects.
// Use this in main.cpp to trigger webServerInit() at the right moment.
bool wasJustConnected();

// Returns current IP as string, or "0.0.0.0" if not connected.
String getIP();

// Returns stored SSID (empty string if nothing saved).
String getStoredSSID();

// JSON-formatted status string, e.g.:
//   {"wifi":"connected","ssid":"LabNet","ip":"192.168.1.22"}
String statusJSON();

} // namespace WiFiManager