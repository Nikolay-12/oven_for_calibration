#ifndef StabilityChecker_h
#define StabilityChecker_h

#include <Arduino.h>

// ==================================================
//  КЛАСС ПРОВЕРКИ СТАБИЛЬНОСТИ
// ==================================================
class StabilityChecker {
private:
    float dt; // шаг по времени между измерениями (с)
    float tolerance; // допустимое отклонение измеряемой температуры от заданной (°C)
    float required_time; // время в течении которого надо находиться в допуске (с)
    float target_temp; // температура, которую необходимо достичь (°C)
    int samples_counter = 0;
    int required_samples = 0; // сколько измерений должно быть в допуске для продолжения работы
    
public:
    StabilityChecker(float dt, float tolerance, float required_time)
        : dt(dt), tolerance(tolerance), required_time(required_time) {
        reset();
        setTarget(0);
        updateRequiredSamples(dt, required_time);
    }

    void updateRequiredSamples(float new_step, float new_total_time){
        required_samples = (int)(new_total_time / new_step) + 1; // изменяем required_samples под пользовательские настройки + 1 
    }

    int getRequiredSamples(){
        return required_samples;
    }

    void setTarget(float new_target){
        target_temp = new_target;
    }
    
    void reset() {
        samples_counter = 0;
    }
    
    void updateStabilityCounter(float temperature) {
        if (abs(temperature - target_temp) <= tolerance) {
            samples_counter++;
        }
        else {
            samples_counter = 0;
        }
    }
    
    bool isStable() const {
        if (samples_counter < required_samples) return false;
        return true;
    }
    
    float getCurrentStability() const {
        float result = 100.0 * samples_counter / required_samples;
        if (samples_counter > required_samples) return 100.0;
        return result;
    }
};

#endif