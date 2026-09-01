#pragma once

#include <Arduino.h>

namespace measurementProcessing
{
    void processMeasurementsIT(float* temperatures, float* currents, int size);
    void processMeasurementsIV(float* currents, int size);
}