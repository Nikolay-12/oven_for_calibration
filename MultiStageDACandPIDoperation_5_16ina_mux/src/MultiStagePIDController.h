#pragma once

#include "project_config.h"
#include "PIDregulator.h"
#include "StabilityChecker.h"


// ==================================================
//  СТРУКТУРА ЭТАПА
// ==================================================
struct ThermoStage {
    float temperature;
    float stability_time;
    float hold_time;
    float tolerance;

    ThermoStage() : temperature(0), stability_time(0), hold_time(0), tolerance(0) {}
    ThermoStage(float temp, float stab_time, float hold_t, float tol)
        : temperature(temp), stability_time(stab_time), hold_time(hold_t), tolerance(tol) {}
    // Параметры: температура (°C), время для стабилизации (с), время на выдержку (с) и температурный допуск (°C)   
};

// ==================================================
//  ОСНОВНОЙ КЛАСС УПРАВЛЕНИЯ
// ==================================================
class MultiStagePIDController {
private:
    enum Mode  {IDLE = 0, 
                MULTI_STAGE = 1, // Многостадийный пользовательский режим
                TEMPERATURE_INCREASING = 2, // Режим поэтапного увеличения напряжения с Vmin до Vmax
                TEMPERATURE_DECREASING = 3}; // Режим поэтапного уменьшения напряжения с Vmax до Vmin
    enum State {STABILIZATION, 
                HOLDING, 
                MEASURING, 
                SWITCHING};
    Mode current_mode;
    State current_state;
    
    // Параметры для режима 1
    ThermoStage* stages;
    int stage_count;
    int current_stage;

    // Параметры для режимов 2 и 3
    float current_temp = 20.0f;
    float required_temp = 20.0f;
    float MIN_TEMP = 20.0f;
    float MAX_TEMP = 100.0f;
    float temp_step = 10.0f;
    
    float stability_time;
    float max_stage_time; 
    float hold_time; 
    float tolerance;
    unsigned long process_start_time;
    unsigned long stage_start_time;
    unsigned long hold_start_time;
    unsigned long last_log_time;
    
    StabilityChecker stability_checker;
    PIDController pid;

    float Kp;
    float Ki;
    float Kd;
    float Dt;
    bool verbose;
    
    // Для измерений
    float (*readTemperatureFunc)();
    void (*readCurrentFunc)();
    
    bool isMeasuring = false;
    int target_cycles = 0;
    int current_cycle = 0;
    float measurement_period = 1.0;
    unsigned long last_measure_time = 0;
    
    // Буферы для данных
    float* temp_buffer = nullptr;
    float* current_buffer = nullptr;
    int buffer_size = 0;
    
    // Колбэк завершения 
    void (*onCompleteFunc)(float*, float*, int) = nullptr;

public:
    MultiStagePIDController(uint8_t mode, float* temperatures, int count, 
                           float stab_time, float hold_time_s, float tolerance, float max_time_per_stage,
                           float kp, float ki, float kd, float dt_s,      
                           bool verbose_output,
                           float (*tempCallback)(), void (*currentCallback)(), 
                           int cycles, float measurement_period, float* temp_buf, float* current_buf, int buf_size,
                           void (*onCompleteCallback)(float*, float*, int)): 
        pid(kp, ki, kd, dt_s),
        stage_count(count),
        current_stage(0),
        stability_time(stab_time), 
        hold_time(hold_time_s), 
        tolerance(tolerance),
        Kp(kp), Ki(ki), Kd(kd), Dt(dt_s),
        current_state(STABILIZATION),
        stability_checker(dt_s, tolerance, stab_time), // параметры: шаг между измерениями (с), допуск по температуре, float required_time
        max_stage_time(max_time_per_stage),
        verbose(verbose_output), // дополнительный параметр, который в состоянии true выдает дополнительную информацию о ходе
                                // работы
        readTemperatureFunc(tempCallback), // функция, вызываемая для измерения температуры
        readCurrentFunc(currentCallback), // функция, вызываемая для измерения тока

        target_cycles(cycles), // количество циклов измерений в конце этапа выдержки
        measurement_period(measurement_period), // время между циклами измерений в конце этапа выдержки
        temp_buffer(temp_buf), // буфер измерений температур в конце этапа выдержки
        current_buffer(current_buf), // буфер измерений токов в конце этапа выдержки
        buffer_size(buf_size),
        onCompleteFunc(onCompleteCallback), // функция, вызываемая по завершению измерения

        last_log_time(0),
        stage_start_time(0),
        hold_start_time(0) {
        
        Mode current_mode = static_cast<Mode>(mode);
        //buffer_size = (buf_size < cycles) ? buf_size : cycles;

        // Создаём этапы с параметрами по умолчанию
        stages = new ThermoStage[count];
        for (int i = 0; i < count; i++) {
            stages[i] = ThermoStage(temperatures[i], stab_time, hold_time_s, tolerance);                                              
        }
        
        //pid.setSetpoint(stages[0].temperature);
        pid.setSetpoint(0);
        stability_checker.reset();
        stability_checker.setTarget(0);  
        //stability_checker.setTarget(stages[0].temperature);    
    }
    
    ~MultiStagePIDController() {
        delete[] stages;
    }

    void reset_mode_state_timers() {
        // Сбрасываем состояние и таймеры
        current_stage = 0;
        current_state = State::STABILIZATION;
        pid.reset();  // Сброс интегральной составляющей
        stability_checker.reset();
        pid.setSetpoint(0);
        stability_checker.setTarget(0);
        process_start_time = 0;
        stage_start_time = 0;
        hold_start_time = 0;
        last_log_time = 0;
    }

    void stages_settings_reset(float* temperatures, int count) {
        current_mode = Mode::MULTI_STAGE;

        // 1. Пересоздание массива этапов
        delete[] stages;  
        stages = new ThermoStage[count];
        stage_count = count;
        for (int i = 0; i < count; i++) {
            stages[i] = ThermoStage(temperatures[i], stability_time, hold_time, tolerance);
        }
        
        // 2. Обновление параметров ПИД-регулятора
        reset_mode_state_timers();
        pid.setSetpoint(stages[0].temperature);
        stability_checker.setTarget(stages[0].temperature);
       
        Serial.println("Стадии многостадийного терморегулятора обновлены!");
        Serial.println("По умолчанию установлен режим MULTI_STAGE.");
    }

    void temp_settings_reset(uint8_t new_mode, float new_Tmin, float new_Tmax, float new_deltaT) {
        MIN_TEMP = new_Tmin;
        MAX_TEMP = new_Tmax;
        temp_step = new_deltaT;
        stage_count = (MAX_TEMP - MIN_TEMP) / temp_step + 1;
        Mode current_mode = static_cast<Mode>(new_mode);
        Serial.println("Настройки для автоматического создания уровней обновлены!");    
    }

    void PID_settings_reset(float new_kp, float new_ki, float new_kd,  float new_dt) {
        Kp = new_kp; Ki = new_ki; Kd = new_kd ; Dt = new_dt;
        reset_mode_state_timers();
        pid.updateSettings(new_kp, new_ki, new_kd, new_dt);
        stability_checker.updateRequiredSamples(Dt, stability_time);
        Serial.println("ПИД-настройки обновлены!");
    }

    void time_and_tol_settings_reset(float new_stab_time, float new_hold_time, float new_tolerance, float max_time_per_stage) {
        // 1. Обновление массива этапов        
        for (int i = 0; i < stage_count; i++) {
            stages[i] = ThermoStage(stages[i].temperature, new_stab_time, new_hold_time, new_tolerance);
        }
        stability_time = new_stab_time;
        hold_time = new_hold_time;
        tolerance = new_tolerance;
        max_stage_time = max_time_per_stage;
        reset_mode_state_timers();
        stability_checker.updateRequiredSamples(Dt, stability_time);
        Serial.println("Временные настройки и погрешность обновлены!");
    }

    void measurement_reset(int new_cycles_num, float new_measurement_period) {
        target_cycles = new_cycles_num; // количество циклов измерений в конце этапа выдержки
        measurement_period = new_measurement_period; // время между циклами измерений в конце этапа выдержки  
        Serial.println("Настройки измерений регулятора переинициализированы!");
    }
    
    bool update(float current_temperature, float& heater_output_percent, float dt_s) {
        unsigned long now = millis();
        float elapsed_stage = (now - stage_start_time) / 1000.0; // прошедшее время стадии (в секундах)
        double heater_output = pid.update(current_temperature); // возвращает управляющий сигнал в диапазоне [0..255]
        heater_output_percent = heater_output / 255 * 100; // возвращает управляющий сигнал в процентах [0..100]
        
        if (current_state == State::STABILIZATION || current_state == State::HOLDING) {
            stability_checker.updateStabilityCounter(current_temperature);
        }
        
        switch (current_state) {
            case STABILIZATION: // работа на этапе стабилизации
                if (stability_checker.isStable()) {
                    current_state = State::HOLDING;
                    hold_start_time = now;
                    if (verbose) {
                        Serial.println("[LOG] Стабилизация достигнута!");
                    }
                }
                else if (elapsed_stage > max_stage_time) {
                    if (verbose) {
                        Serial.println("[LOG] Таймаут этапа! Принудительный переход.");
                    }
                    current_state = State::HOLDING;
                    hold_start_time = now;
                }
                break;
                
            case HOLDING: { // работа на этапе выдержки
                if (now - hold_start_time >= hold_time * 1000) {
                    if (verbose) {
                        Serial.println("[LOG] Выдержка завершена.");
                    }
                    current_state = State::MEASURING;
                    startMeasurements(); // Запуск измерений
                }
                break;
            }
            case MEASURING: { // работа на этапе измерения
                updateMeasurements(); 
                break;
            }

            case SWITCHING: // работа на этапе переключения
                if (current_stage + 1 < stage_count) {
                    current_stage++;
                    current_state = State::STABILIZATION;
                    stability_checker.reset();
                    switch (current_mode) {
                        case Mode::MULTI_STAGE: // Режим 1.
                            required_temp = stages[current_stage].temperature;
                            break;
                        case Mode::TEMPERATURE_INCREASING: // Режим 2.
                            required_temp += temp_step;
                            break;
                        case Mode::TEMPERATURE_DECREASING: // Режим 3.
                            required_temp -= temp_step;
                            break;
                    }
                    pid.setSetpoint(required_temp);
                    stability_checker.setTarget(required_temp);
                    stage_start_time = now;
                    
                    if (verbose) {
                        Serial.print("[LOG] Переход к этапу ");
                        Serial.print(current_stage + 1);
                        Serial.print(": ");
                        Serial.print(required_temp);
                        Serial.println(" °C");
                    }
                } 
                else {
                    scriptEnding();
                    return false;
                }
                break;
        }
        
        // Логирование раз в секунду
        if (verbose && (now - last_log_time) >= 1000) {
            logStatus(current_temperature, heater_output_percent); // heater_output
            last_log_time = now;
        }
        
        return true;
    }

    void scriptStarting(uint8_t mode) {
        if ((mode == 1) || (mode == 2) || (mode == 3)) {
            Serial.println("The script execution was started...");
            current_mode = static_cast<Mode>(mode);
            // Инициализация для нового режима
            current_state = State::STABILIZATION;
            switch (current_mode) {
                case Mode::MULTI_STAGE:
                    // Инициализация многостадийного режима
                    required_temp = stages[current_stage].temperature;
                    Serial.println("The temperature is set to the first stage.");
                    break;
                case Mode::TEMPERATURE_INCREASING:
                    required_temp = MIN_TEMP;
                    Serial.println("The temperature is set to Tmin.");
                    break;
                case Mode::TEMPERATURE_DECREASING:
                    required_temp = MAX_TEMP;
                    Serial.println("The temperature is set to Tmax.");
                    break;
            }
            current_stage = 0;
            pid.setSetpoint(required_temp);
            stability_checker.setTarget(required_temp);
            process_start_time = millis();
            stage_start_time = millis();
            hold_start_time = millis();
        }
        else {
            Serial.println("There is no such mode!");
        }
    }
    void scriptEnding() {
        if ((current_mode == Mode::MULTI_STAGE) || (current_mode == Mode::TEMPERATURE_INCREASING) || (current_mode == Mode::TEMPERATURE_DECREASING)) {
            Serial.println("The script execution was completed.");
            required_temp = 0;
            pid.setSetpoint(required_temp);
            stability_checker.setTarget(required_temp);
            Serial.println("The temperature is set to 0.");
        }
    }

    // Запуск измерений
    void startMeasurements() {
        if (isMeasuring) return;
        
        last_measure_time = millis();
        isMeasuring = true;
        resetMeasurementsData();
        
        Serial.print("The measurements have started (");
        Serial.print(target_cycles);
        Serial.println(" cycles)");
    }
    
    // Обновление данных с датчиков на этапе MEASURING
    void updateMeasurements() {
        if (!isMeasuring) return;
        
        unsigned long now = millis();
        if ((now - last_measure_time) >= (measurement_period * 1000)) {
            last_measure_time = now;
            
            // Используем указатели на функции для чтения
            if (temp_buffer && current_buffer && current_cycle < buffer_size) {
                temp_buffer[current_cycle] = readTemperatureFunc();
                //current_buffer[current_cycle] = readCurrentFunc();
                logMeasurement(current_cycle);
                current_cycle++;
            }
            
            // Проверка завершения
            if (current_cycle >= target_cycles) {
                isMeasuring = false;
                Serial.println("Measurements complete!");
                
                if (onCompleteFunc) {
                    onCompleteFunc(temp_buffer, current_buffer, current_cycle);
                }
                current_state = State::SWITCHING;
            }
        }
    }

    void resetMeasurementsData() {
        for (int i = 0; i < buffer_size; i++) {
            temp_buffer[i] = 0;
            current_buffer[i] = 0;
        }
        current_cycle = 0;
    }
    
    bool isMeasuringInProgress() const { return isMeasuring; }
    
private:
    // Логирование для стадии измерений (MEASURING)
    void logMeasurement(int index) {
        Serial.print("[MEASURING] (");
        Serial.print(index + 1);
        Serial.print("/");
        Serial.print(target_cycles);
        Serial.print(") T = ");
        Serial.print(temp_buffer[index]);
        Serial.print(" °C | I = ");
        //Serial.print(current_buffer[index]);
        readCurrentFunc();
        Serial.println(" mA");
    }

    // Логирование для стадий работы терморегулятора, кроме стадии измерений (MEASURING)
    void logStatus(float temp, float output) {
        switch (current_state) {
            case STABILIZATION: Serial.print("[STABILIZATION]"); break;
            case HOLDING:  Serial.print("[HOLDING]"); break;
            //case MEASURING:  Serial.print("MEASURING"); break;
            case SWITCHING: Serial.println("[STAGE SWITCHING]"); break;
        }
        if ((current_state == STABILIZATION) || (current_state == HOLDING)) {
            Serial.print(" current T = ");
            Serial.print(temp, 1);
            Serial.print(" °C | Setpoint = ");
            Serial.print(pid.getSetpoint(), 0);
            Serial.print(" °C | Heater power = ");
            Serial.print(output, 0);
            Serial.println(" %");
        }
    }
};