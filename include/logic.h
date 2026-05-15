#pragma once
#include "sensors.h"
struct SystemState {
    bool falla;
    bool HornoValido;
};

SystemState evaluarSistema(SensorData data);