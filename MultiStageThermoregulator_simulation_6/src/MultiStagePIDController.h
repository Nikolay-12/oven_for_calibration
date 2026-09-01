#ifndef MultiStagePIDController_h
#define MultiStagePIDController_h

#include <Arduino.h>
#include "PIDregulator.h"
#include "StabilityChecker.h"
// ==================================================
//  СТРУКТУРА ЭТАПА
// ==================================================
struct Stage {
    float temperature;
    float stability_time;
    float hold_time;
    float tolerance;

    Stage() : temperature(0), stability_time(0), hold_time(0), tolerance(0) {}
    Stage(float temp, float stab_time, float hold_t, float tol)
        : temperature(temp), stability_time(stab_time), hold_time(hold_t), tolerance(tol) {}
};

// ==================================================
//  ОСНОВНОЙ КЛАСС УПРАВЛЕНИЯ
// ==================================================
class MultiStagePIDController {
private:
    PIDController pid;
    Stage* stages;
    int stage_count;
    int current_stage;
    
    enum State { STABILIZATION, HOLDING, SWITCHING };
    State state;
    
    double stability_time; 
    double hold_time; 
    double tolerance;
    unsigned long stage_start_time;
    unsigned long hold_start_time;
    unsigned long last_log_time;
    
    StabilityChecker stability_checker;
    
    float dt;
    float max_stage_time;
    bool verbose;
    
public:
    MultiStagePIDController(float* temperatures, int count, 
                           float stability_time, float hold_time, float tolerance, float max_time_per_stage,
                           float kp, float ki, float kd, float dt_sec,                          
                           bool verbose_output)
        : pid(kp, ki, kd, dt_sec),
          stage_count(count),
          current_stage(0),
          stability_time(stability_time), 
          hold_time(hold_time), 
          tolerance(tolerance),
          state(STABILIZATION),
          stability_checker(dt_sec, tolerance, stability_time), // параметры: шаг между измерениями (с), допуск по температуре, float required_time
          dt(dt_sec),
          max_stage_time(max_time_per_stage),
          verbose(verbose_output), // дополнительный параметр, который в состоянии true выдает дополнительную информацию о ходе
                                   // работы
          last_log_time(0),
          stage_start_time(0),
          hold_start_time(0) {
        
        // Создаём этапы с параметрами по умолчанию
        stages = new Stage[count];
        for (int i = 0; i < count; i++) {
            stages[i] = Stage(temperatures[i], stability_time, hold_time, tolerance); // параметры: температура (°C), время для 
                                                                                    // стабилизации (с), время на выдержку (с)
                                                                                    // и температурный допуск (°C)                                                
        }
        
        pid.setSetpoint(stages[0].temperature);
        stability_checker.reset();
        stability_checker.setTarget(stages[0].temperature);    
    }
    
    ~MultiStagePIDController() {
        delete[] stages;
    }

    void reset(float* temperatures, int count, 
               float new_stability_time, float new_hold_time, float new_tolerance, float max_time_per_stage,
               float new_kp, float new_ki, float new_kd, float new_dt,
               bool verbose_output) {
             
        // 1. Пересоздание массива этапов
        delete[] stages;  
        stages = new Stage[count];
        stage_count = count;  
        for (int i = 0; i < count; i++) {
            stages[i] = Stage(temperatures[i], new_stability_time, new_hold_time, new_tolerance);
        }

        // 2. Обновление параметров ПИД-регулятора
        pid.updateSettings(new_kp, new_ki, new_kd, new_dt);
        pid.setSetpoint(stages[0].temperature);
        pid.reset();  // Сброс интегральной составляющей
        
        // 3. Сбрасываем состояние и таймеры
        current_stage = 0;
        state = STABILIZATION;
        stability_checker.reset();
        stability_checker.setTarget(stages[0].temperature);
        stage_start_time = millis();
        hold_start_time = 0;
        last_log_time = 0;
        
        Serial.println("Регулятор переинициализирован!");
    }
    
    bool update(float current_temperature, float& heater_output_percent, float dt_s) {
        unsigned long now = millis();
        float elapsed_stage = (now - stage_start_time) / 1000.0; // прошедшее время стадии (в секундах)
        double heater_output = pid.update(current_temperature); // возвращает управляющий сигнал в диапазоне [0..255]
        heater_output_percent = heater_output / 255 * 100; // возвращает управляющий сигнал в процентах [0..100]
        
        if (state == STABILIZATION || state == HOLDING) {
            stability_checker.updateStabilityCounter(current_temperature);
        }
        
        switch (state) {
            case STABILIZATION: // работа на этапе стабилизации
                if (stability_checker.isStable()) {
                    state = HOLDING;
                    hold_start_time = now;
                    if (verbose) {
                        Serial.println("[LOG] Стабилизация достигнута!");
                    }
                }
                else if (elapsed_stage > max_stage_time) {
                    if (verbose) {
                        Serial.println("[LOG] Таймаут этапа! Принудительный переход.");
                    }
                    state = HOLDING;
                    hold_start_time = now;
                }
                break;
                
            case HOLDING: { // работа на этапе выдержки
                float elapsed_hold = (now - hold_start_time) / 1000.0;
                if (elapsed_hold >= stages[current_stage].hold_time) {
                    if (verbose) {
                        Serial.println("[LOG] Выдержка завершена.");
                    }
                    state = SWITCHING;
                }
                break;
            }
                
            case SWITCHING: // работа на этапе переключения
                if (current_stage + 1 < stage_count) {
                    current_stage++;
                    state = STABILIZATION;
                    pid.setSetpoint(stages[current_stage].temperature);
                    stability_checker.reset();
                    stability_checker.setTarget(stages[current_stage].temperature);
                    stability_checker.updateRequiredSamples(dt_s, stages[current_stage].stability_time);
                    stage_start_time = now;
                    
                    if (verbose) {
                        Serial.print("[LOG] Переход к этапу ");
                        Serial.print(current_stage + 1);
                        Serial.print(": ");
                        Serial.print(stages[current_stage].temperature);
                        Serial.println("°C");
                    }
                } else {
                    return false;
                }
                break;
        }
        
        // Логирование раз в секунду
        if (verbose && (now - last_log_time) >= 1000) {
            logStatus(current_temperature, heater_output);
            last_log_time = now;
        }
        
        return true;
    }
    
    void run(float (*readTemperature)(), void (*setHeaterOutput)(float)) {
        stage_start_time = millis();
        
        if (verbose) {
            Serial.println("========================================");
            Serial.println("   ЗАПУСК МНОГОСТАДИЙНОГО ПИД-РЕГУЛЯТОРА");
            Serial.println("========================================");
            for (int i = 0; i < stage_count; i++) {
                Serial.print("Этап ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(stages[i].temperature);
                Serial.println("°C");
            }
            Serial.println();
        }
        
        float heater_output = 0.0;
        unsigned long last_update = 0;
        
        while (true) {
            unsigned long now = millis();
            if ((now - last_update) >= (dt * 1000)) {
                float temp = readTemperature();
                bool running = update(temp, heater_output, dt);
                setHeaterOutput(heater_output);
                
                if (!running) {
                    if (verbose) {
                        Serial.println("\n[LOG] Все этапы завершены!");
                        setHeaterOutput(0.0);
                    }
                    break;
                }
                last_update = now;
            }
            delay(1); // Небольшая задержка для стабильности
        }
    }
    
private:
    void logStatus(float temp, float output) {
        Serial.print("[");
        switch (state) {
            case STABILIZATION: Serial.print("СТАБИЛИЗАЦИЯ"); break;
            case HOLDING:  Serial.print("ВЫДЕРЖКА"); break;
            case SWITCHING: Serial.print("СМЕНА ЭТАПА"); break;
        }
        Serial.print("] T=");
        Serial.print(temp, 1);
        Serial.print(" °C | Уст=");
        Serial.print(pid.getSetpoint(), 0);
        Serial.print(" °C | Нагрев=");
        float output_percent = output / 255 * 100;
        Serial.print(output_percent, 0);
        Serial.println(" %");
    }
};

#endif