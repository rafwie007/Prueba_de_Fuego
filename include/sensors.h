#pragma once
#define Num_Sensores 2

struct SensorData {
    float tempsHorno[Num_Sensores];
    float tempProm;
    float delta;
    float tempPlaca;
};

void sensorsInit();
SensorData readSensors();