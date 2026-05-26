#pragma once
#include "sensor_manager.h"

// ── Web Server ────────────────────────────────────────────────────────────────
// AsyncWebServer that serves the dashboard UI and SSE data stream.
//
// CHANGED from original:
//   • webServerInit() takes NO credentials — WiFiManager owns the connection.
//   • Call webServerInit() once in setup(); it starts serving immediately.
//     The server works over AP or STA depending on WiFiManager state.
// ─────────────────────────────────────────────────────────────────────────────

// Start the async web server (call once in setup).
void webServerInit();

// Push a new sensor snapshot to all connected SSE clients.
void webServerPushData(const SensorData& data, const SystemState& state);