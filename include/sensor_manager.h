#pragma once
#include <Arduino.h>
#include "sensors.h"
#include "logic.h"

// ── SensorManager ─────────────────────────────────────────────────────────────
// Thin facade over the existing sensors.h / logic.h modules.
//
//   • Holds the latest SensorData and SystemState snapshots.
//   • Provides JSON status string consumed by the serial CLI and web server.
//   • Controls a serial logging flag (toggled by "log start / log stop").
//   • The actual 1-second read loop lives in main.cpp; call update() there.
// ─────────────────────────────────────────────────────────────────────────────

namespace SensorManager {

// Call every time new data is available (replaces direct readSensors() calls).
void update();

// Enable / disable continuous JSON logging to serial.
void setLogging(bool enabled);
bool isLogging();

// Returns the most recent readings as a JSON string, e.g.:
// {"t1":45.2,"t2":44.8,"prom":45.0,"delta":0.4,"placa":23.1,"running":true}
String statusJSON();

// Accessor for the last snapshot (used by webserver push).
SensorData    lastData();
SystemState   lastState();

} // namespace SensorManager