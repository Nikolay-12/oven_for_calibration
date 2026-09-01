#ifndef ThermalModel_h
#define ThermalModel_h

#include <Arduino.h>

class ThermalModel {
private:
    float temperature; // текущая температура
    float ambient_temp; // комнатная температура
    float thermal_capacity; // теплоемкость
    float thermal_resistance; // термическое сопротивление 
    float heater_power_max; // коэффициент мощности нагревателя
    float noise_amplitude; // амплитуда шума (симуляция реальной термопары)
    unsigned long last_update;
    
public:
    ThermalModel(float ambient = 25.0, float capacity = 0.95, 
                 float resistance = 20.0, float max_power = 10.0,
                 float noise = 0.2)
        : ambient_temp(ambient), thermal_capacity(capacity),
          thermal_resistance(resistance), heater_power_max(max_power),
          noise_amplitude(noise), last_update(0) {
        temperature = ambient;
    }
    
    float update_temp(float heater_output_percent, float dt) {
        float heater_power = (heater_output_percent / 100.0) * heater_power_max;
        float power_loss = (temperature - ambient_temp) / thermal_resistance;
        float dT = (heater_power - power_loss) * dt / thermal_capacity;
        temperature += dT;
        
        // Простой шум (случайное значение)
        float noise = ((float)random(-100, 100) / 100.0) * noise_amplitude;
        float measured = temperature + noise;
        
        if (measured < ambient_temp - 10) measured = ambient_temp - 10;
        return measured;
    }
    
    float getTemperature() const { return temperature; }
    void setTemperature(float temp) { temperature = temp; }
};

#endif