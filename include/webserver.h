#pragma once
#include "sensors.h"
#include "logic.h"
#include "secrets.h"

void webServerInit(const char* ssid, const char* password);
// webServerHandle() ya no es necesario — AsyncWebServer es no-bloqueante
void webServerPushData(SensorData data, SystemState state);