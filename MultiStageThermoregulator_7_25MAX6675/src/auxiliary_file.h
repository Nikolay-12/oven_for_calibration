#pragma once
#include <cmath>

bool isDivisible(float a, float b, float epsilon = 1e-6) {
    if (fabs(b) < epsilon) return false; // Защита от деления на ноль
    
    float remainder = fmod(a, b);
    return fabs(remainder) < epsilon || fabs(remainder - b) < epsilon;
}