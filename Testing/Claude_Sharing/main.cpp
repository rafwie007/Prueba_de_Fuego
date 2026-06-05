#include <Arduino.h>
#include "serial_commands.h"   // non-blocking serial CLI
#include "wifi_manager.h"      // NVS-backed WiFi (no hardcoded credentials)
#include "sensor_manager.h"    // sensors + logic facade
#include "webserver.h"         // async HTTP + SSE dashboard

static const unsigned long SENSOR_INTERVAL_MS = 1000UL;
static bool _webServerRunning = false;

void setup() {
    // 1. Serial CLI first — visible immediately, works with no WiFi at all.
    SerialCLI::begin(115200);

    // 2. WiFi — loads NVS credentials and starts connecting if found.
    //    webServerInit() is NOT called here; the lwIP stack isn't ready
    //    until WiFi actually associates, and calling it early causes a
    //    hard assert crash (tcpip_api_call / Invalid mbox).
    WiFiManager::begin();

    // 3. Sensors
    sensorsInit();

    Serial.println("{\"setup\":\"complete\",\"webserver\":\"waiting_for_wifi\"}");
}

void loop() {
    // ── Serial CLI — always available, even with no WiFi ─────────────────────
    SerialCLI::handle();

    // ── WiFi reconnect manager ────────────────────────────────────────────────
    WiFiManager::handle();

    // ── Start web server exactly once, on the tick WiFi first connects ────────
    if (!_webServerRunning && WiFiManager::wasJustConnected()) {
        webServerInit();
        _webServerRunning = true;
    }

    // ── Sensor read + web push (1 Hz) ─────────────────────────────────────────
    static unsigned long _lastRead = 0;
    if (millis() - _lastRead >= SENSOR_INTERVAL_MS) {
        _lastRead = millis();
        SensorManager::update();
        if (_webServerRunning) {
            webServerPushData(SensorManager::lastData(), SensorManager::lastState());
        }
    }
}