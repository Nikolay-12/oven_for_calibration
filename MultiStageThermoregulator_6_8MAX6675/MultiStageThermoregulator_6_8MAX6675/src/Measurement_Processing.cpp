#include "Measurement_Processing.h"

namespace measurementProcessing
{
    void processMeasurementsIT(float* temps, float* currents, int count) {
        Serial.println("=== PROCESSING RESULTS ===");
        
        // Вычисляем средние значения
        float tempSum = 0, currentSum = 0;
        for (int i = 0; i < count; i++) {
            tempSum += temps[i];
            currentSum += currents[i];
        }
        
        float tempAvg = tempSum / count;
        float currentAvg = currentSum / count;
        
        Serial.print("Average Temperature: ");
        Serial.print(tempAvg, 1);
        Serial.println("°C");
        Serial.print("Average Current: ");
        Serial.print(currentAvg, 2);
        Serial.println(" mA");
        
        Serial.println("=== READY FOR NEXT STEP ===");
    }
}
