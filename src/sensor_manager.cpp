#include "sensor_manager.h"
#include "output.h"
#include <Arduino.h>
#include "serial_commands.h"

// ── Module-private state ──────────────────────────────────────────────────────
static SensorData   _lastData  = {};
static SystemState  _lastState = {};
static bool         _logging   = false;

// ── Public API ────────────────────────────────────────────────────────────────
namespace SensorManager {

void update() {
    _lastData  = readSensors();
    _lastState = evaluarSistema(_lastData);
  // outputData(_lastData, _lastState);      // keeps existing output.h behaviour

    if (_logging) {
        String s = statusJSON();
        // send via interruptPrint to avoid clobbering CLI input
        SerialCLI::interruptPrint(s.c_str());
    }
}

void setLogging(bool enabled) {
    _logging = enabled;
}

bool isLogging() {
    return _logging;
}

String statusJSON() {
    char buf[192];
    snprintf(buf, sizeof(buf),
        "{"
        "\"t1\":%.2f,"
        "\"t2\":%.2f,"
        "\"prom\":%.2f,"
        "\"delta\":%.2f,"
        "\"placa\":%.2f,"
        "\"falla\":%s,"
        "\"hornoValido\":%s"
        "}",
        _lastData.tempsHorno[0],
        _lastData.tempsHorno[1],
        _lastData.tempProm,
        _lastData.delta,
        _lastData.tempPlaca,
        _lastState.falla       ? "true" : "false",
        _lastState.HornoValido ? "true" : "false"
    );
    return String(buf);
}

SensorData lastData()     { return _lastData;  }
SystemState lastState()   { return _lastState; }

} // namespace SensorManager