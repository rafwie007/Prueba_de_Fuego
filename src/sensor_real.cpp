#include <Arduino.h>
#include "sensors.h"
#include <max6675.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#define PIN_SCK  18
#define PIN_SO   19

#define PIN_CS1  5    // Horno sensor 1
#define PIN_CS2  17   // Horno sensor 2

MAX6675 sensor1(PIN_SCK, PIN_CS1, PIN_SO);
MAX6675 sensor2(PIN_SCK, PIN_CS2, PIN_SO);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void sensorsInit() {
    delay(500); 

    Wire.begin(21, 22); // SDA, SCL (ESP32 default)

    if (!mlx.begin()) {
        Serial.println("Error: MLX90614 not detected!");
    }
}

SensorData readSensors() {
    SensorData data;

    float t1 = sensor1.readCelsius();
    float t2 = sensor2.readCelsius();

    data.tempsHorno[0] = t1;
    data.tempsHorno[1] = t2;
    data.tempProm      = (t1 + t2) / 2.0;
    data.delta         = abs(t1 - t2);

// EMA (Exponential Moving Average) para suavizar la lectura de la placa
    static float placaFiltered = 0;
    float raw = mlx.readObjectTempC();
    placaFiltered = 0.8 * placaFiltered + 0.2 * raw;
    data.tempPlaca = placaFiltered;

    return data;
}
