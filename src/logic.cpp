#include "logic.h"

SystemState evaluarSistema(SensorData data) {
    SystemState state;

    state.falla = data.tempPlaca >= 180;
    state.HornoValido = data.delta <= 25;

    return state;
}