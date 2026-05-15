#include <Arduino.h>
#include "output.h"

unsigned long startTime;

void outputInit() {
    Serial.begin(115200);
    delay(1000);
    startTime = millis();
    Serial.println("Preuba de Fuego");
}

void outputData(SensorData data, SystemState state) {
  float tiempo_min = (millis() - startTime) / 60000.0;
  Serial.print(tiempo_min);
  Serial.print(",");
  Serial.print(data.tempsHorno[0]);
  Serial.print(",");
  Serial.print(data.tempsHorno[1]);
  Serial.print(",");
  Serial.print(data.tempProm);
  Serial.print(",");
  Serial.print(data.delta);
  Serial.print(",");
  Serial.println(data.tempPlaca);
 }