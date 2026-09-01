#pragma once

#include <cmath>
#include "project_config.h"

extern bool i2cInitialized;

void initI2C(uint8_t SDA_PIN, uint8_t SCL_PIN);


bool isDivisible(float a, float b);