#include "auxiliary_file.h"

bool i2cInitialized = false;

void initI2C(uint8_t SDA_PIN, uint8_t SCL_PIN) {
  if (!i2cInitialized) { // Инициализация шины I2C, если она ещё не инициализирована
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    i2cInitialized = true;
    Serial.println("I2C инициализирована.");
  }
}


bool isDivisible(float a, float b) {
    float epsilon = 1e-6;
    if (fabs(b) < epsilon) return false; // Защита от деления на ноль
    
    float remainder = fmod(a, b);
    return fabs(remainder) < epsilon || fabs(remainder - b) < epsilon;
}