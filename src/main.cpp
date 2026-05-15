#include <Arduino.h>
#include "sensors.h"
#include "logic.h"
#include "output.h"
#include "webserver.h"

void setup() {
    outputInit();
    sensorsInit();
    webServerInit(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    // AsyncWebServer es no-bloqueante — no necesita webServerHandle()

    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 1000) {
        lastRead = millis();

        SensorData data   = readSensors();
        SystemState state = evaluarSistema(data);
        outputData(data, state);
        webServerPushData(data, state);
    }
}