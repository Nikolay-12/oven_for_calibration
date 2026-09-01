#pragma once

#include "project_config.h"

// ==================================================
//  СТРУКТУРА ЭТАПА
// ==================================================
struct Stage {
    int level;
    float hold_time;

    Stage() : level(0), hold_time(0) {}
    Stage(int lvl, float hold_t)
        : level(lvl),  hold_time(hold_t) {}
};

// ==================================================
//  ОСНОВНОЙ КЛАСС УПРАВЛЕНИЯ
// ==================================================
class MultiStageDACcontroller {
private:
    enum Mode  {IDLE = 0, 
                MULTI_STAGE = 1, // Многостадийный пользовательский режим
                VOLTAGE_INCREASING = 2, // Режим поэтапного увеличения напряжения с Vmin до Vmax
                VOLTAGE_DECREASING = 3}; // Режим поэтапного уменьшения напряжения с Vmax до Vmin
    enum State {HOLDING, 
                MEASURING, 
                SWITCHING,
                COMPLETED};
    Mode current_mode;
    State current_state;

    // Параметры для режима 1
    Stage* stages;
    int stage_count;
    int current_stage;

    // Параметры для режимов 2 и 3
    unsigned long step_timer;
    int required_level;
    int current_level;
    int MIN_LEVEL = 0;
    int MAX_LEVEL = 4095;
    int level_step = 10;
    float time_step = 1.0; // секунды
    
    float required_voltage;
    float current_voltage;
    float process_time; 
    float hold_time; 
    unsigned long process_start_time;
    unsigned long stage_start_time;
    unsigned long hold_start_time;
    unsigned long last_log_time;
    //float dt;
    bool verbose;
    
    // Для измерений
    void (*setLevelFunc)(size_t, bool);
    float (*readCurrentFunc)();
    
    bool justStarted = false;
    bool isMeasuring = false;
    int target_cycles = 10;
    int current_cycle = 0;
    float measurement_period = 1.0;
    unsigned long last_measure_time = 0;
    
    // Буферы для данных
    float* current_buffer = nullptr;
    int buffer_size = 0;
    
    // Колбэк завершения 
    void (*onCompleteFunc)(float*, int) = nullptr;

public:
//MultiStageDACcontroller() : currentMode(Mode::IDLE), currentStage(0), 
//                     stageCount(0), currentLevel(0) {}
    MultiStageDACcontroller(uint8_t mode, float* volts, int count, float holding_time,    
                        bool verbose_output,
                        void (*setLevel)(size_t level, bool save), float (*currentCallback)(), 
                        int cycles, float measurement_period_s, float* current_buf, int buf_size,
                        void (*onCompleteCallback)(float*, int)): //float dt_sec,   
        stage_count(count),
        current_stage(0),
        hold_time(holding_time), 
        current_state(HOLDING),
        verbose(verbose_output), // дополнительный параметр, который в состоянии true выдает дополнительную информацию о ходе
                                // работы
        required_level(0),
        required_voltage(0), 
        setLevelFunc(setLevel), // функция, вызываемая для выставления логического уровня на ЦАП
        readCurrentFunc(currentCallback), // функция, вызываемая для измерения тока

        target_cycles(cycles), // количество циклов измерений в конце этапа выдержки
        measurement_period(measurement_period_s), // время между циклами измерений в конце этапа выдержки
        current_buffer(current_buf), // буфер измерений токов в конце этапа выдержки
        buffer_size(buf_size),
        onCompleteFunc(onCompleteCallback), // функция, вызываемая по завершению измерения

        process_start_time(0),
        stage_start_time(0),
        hold_start_time(0),
        last_log_time(0)
        {

        Mode current_mode = static_cast<Mode>(mode);
        //buffer_size = (buf_size < cycles) ? buf_size : cycles;

        // Создаём этапы с параметрами по умолчанию
        stages = new Stage[count];
        for (int i = 0; i < count; i++) {
            int new_level = round(volts[i] / Vref * (float)MAX_VALUE);
            stages[i] = Stage(new_level, hold_time); // параметры: напряжение (В), время на выдержку (с)                                           
        }
        setLevelFunc(required_level, false);
    }
    
    ~MultiStageDACcontroller() {
        delete[] stages;
    }

    void reset_stages(const float* volts, int count) {
        current_mode = Mode::MULTI_STAGE;

        // 1. Пересоздание массива этапов
        delete[] stages;  
        stages = new Stage[count];
        stage_count = count;
        for (int i = 0; i < count; i++) {
            int new_level = round(volts[i] / Vref * (float)MAX_VALUE);
            stages[i] = Stage(new_level, hold_time);
        }
        setLevelFunc(stages[0].level, false);
        
        // 3. Сбрасываем состояние и таймеры
        current_stage = 0;
        current_state = State::HOLDING;
        process_start_time = 0;
        //stage_start_time = millis();
        hold_start_time = 0;
        last_log_time = 0;
        
        Serial.println("Стадии многостадийного ЦАП обновлены!");
    }
    void reset_level_settings(uint8_t new_mode, int new_MIN_LEVEL, int new_MAX_LEVEL, int new_LEVEL_STEP) {
        if ((new_MAX_LEVEL - new_MIN_LEVEL) % new_LEVEL_STEP == 0) {
            stage_count = (new_MAX_LEVEL - new_MIN_LEVEL) / new_LEVEL_STEP + 1;
            MIN_LEVEL = new_MIN_LEVEL;
            MAX_LEVEL = new_MAX_LEVEL;
            level_step = new_LEVEL_STEP;
            Mode current_mode = static_cast<Mode>(new_mode);
            Serial.println("Настройки уровней обновлены!");
        }
        else {
            Serial.println("Произошла ошибка в настройке уровней!");
            stage_count = 0;
            MIN_LEVEL = 0;
            MAX_LEVEL = 4095;
        }
    }
    void reset_time_settings(float new_holding_time) {
        hold_time = new_holding_time;
        Serial.println("Время выдержки обновлено!");
    }
    void reset_mode(uint8_t new_mode) {
        Mode current_mode = static_cast<Mode>(new_mode);
    }

    void measurement_reset(int new_cycles_num, float new_measurement_period) {
        target_cycles = new_cycles_num; // количество циклов измерений в конце этапа выдержки
        measurement_period = new_measurement_period; // время между циклами измерений в конце этапа выдержки
        Serial.println("Настройки измерений регулятора переинициализированы!");
    }
    
    bool update(uint8_t mode, int* req_level) {
        // Выполняем соответствующий режим
        bool result = false; 
        unsigned long now = millis();

        //float elapsed_stage = (now - stage_start_time) / 1000.0; // прошедшее время со старта стадии (в секундах)

        switch (current_state) {  
            case State::HOLDING: { // работа на этапе выдержки
                //float elapsed_hold = (now - hold_start_time) / 1000.0; // прошедшее время с начала выдержки (в секундах)
                if (now - hold_start_time >= hold_time * 1000) {
                    if (verbose) {
                        Serial.println("[LOG] Выдержка завершена.");
                    }
                    current_state = State::MEASURING;
                    startMeasurements(); // Запуск измерений
                }
                break;
            }
            case State::MEASURING: { // работа на этапе измерения
                updateMeasurements(); 
                break;
            }
            case State::SWITCHING: { // работа на этапе переключения
                if (current_stage + 1 < stage_count) {
                    current_stage++;
                    current_state = State::HOLDING;
                    switch (current_mode) {
                        case Mode::MULTI_STAGE: // Режим 1.
                            required_level = stages[current_stage].level;
                            break;
                        case Mode::VOLTAGE_INCREASING: // Режим 2.
                            required_level += level_step;
                            break;
                        case Mode::VOLTAGE_DECREASING: // Режим 3.
                            required_level -= level_step;
                            break;
                    }
                    *req_level = required_level;
                    setLevelFunc(required_level, false);
                    hold_start_time = now;

                    if (verbose) {
                        Serial.print("[LOG] Переход к этапу ");
                        Serial.print(current_stage + 1);
                        required_voltage = (float)required_level / MAX_VALUE * Vref;
                        updateCurrentLevelAndVoltge();
                        Serial.print(": next level = "); Serial.print(required_level);
                        Serial.print(" ; next voltage = "); Serial.print(required_voltage); Serial.println(" V");
                        Serial.print(". The voltage on pin ");
                        Serial.print(output_DAC_pin);
                        Serial.print(" is ");
                        Serial.print(current_voltage);
                        Serial.println(" V.");
                    }
                }

                else {
                    scriptEnding();
                    return false;
                }
                break;
            }
        }
        
        // Логирование раз в секунду
        if (verbose && (now - last_log_time) >= 1000) {
            logStatus();
            last_log_time = now;
        }

        return true;
    }

    void scriptStarting(uint8_t mode, int* req_level) {
        if ((mode == 1) || (mode == 2) || (mode == 3)) {
            Serial.println("The script execution was started...");
            current_mode = static_cast<Mode>(mode);
            // Инициализация для нового режима
            current_state = State::HOLDING;
            switch (current_mode) {
                case Mode::MULTI_STAGE:
                    // Инициализация многостадийного режима
                    required_level = stages[current_stage].level;
                    Serial.println("The output voltage of the DAC is set to the first level.");
                    break;
                case Mode::VOLTAGE_INCREASING:
                    required_level = MIN_LEVEL;
                    Serial.println("The output voltage of the DAC is set to Vmin.");
                    break;
                case Mode::VOLTAGE_DECREASING:
                    required_level = MAX_LEVEL;
                    Serial.println("The output voltage of the DAC is set to Vmax.");
                    break;
                //default:
                //    return false;
            }
            current_stage = 0;
            *req_level = required_level;
            setLevelFunc(required_level, false);
            process_start_time = millis();
            hold_start_time = millis();
            justStarted = false;
        }
        else {
            Serial.println("There is no such mode!");
        }
        /*
        Проверка изменения режима
        if (mode != static_cast<int>(current_mode)) {
        }
        */
    }
    void scriptEnding() {
        if ((current_mode == Mode::MULTI_STAGE) || (current_mode == Mode::VOLTAGE_INCREASING) || (current_mode == Mode::VOLTAGE_DECREASING)) {
            Serial.println("The script execution was completed.");
            required_level = 0;
            setLevelFunc(required_level, false);
            Serial.println("The output voltage of the DAC is set to 0.");
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
        Serial.print("Required level = "); Serial.print(required_level); Serial.print(" ; ");
        required_voltage = (float)required_level / MAX_VALUE * Vref;
        Serial.print("required voltage = "); Serial.print(required_voltage); Serial.println(" V");
        updateCurrentLevelAndVoltge();
        Serial.print("Current level = "); Serial.print(current_level); Serial.print(" ; ");
        Serial.print("Current voltage = "); Serial.print(current_voltage); Serial.println(" V");
    }
    
    // Обновление данных с датчиков на этапе MEASURING
    void updateMeasurements() {
        if (!isMeasuring) return;
        
        unsigned long now = millis();
        if ((now - last_measure_time) >= (measurement_period * 1000)) {
            last_measure_time = now;
            
            // Используем указатели на функции для чтения
            if (current_buffer && current_cycle < buffer_size) {
                current_buffer[current_cycle] = readCurrentFunc();
                logMeasurement(current_cycle);
                current_cycle++;
            }
            
            // Проверка завершения
            if (current_cycle >= target_cycles) {
                isMeasuring = false;
                Serial.println("Measurements complete!");
                
                if (onCompleteFunc) {
                    onCompleteFunc(current_buffer, current_cycle);
                }
                current_state = State::SWITCHING;
            }
        }
    }

    void resetMeasurementsData() {
        for (int i = 0; i < buffer_size; i++) {
            current_buffer[i] = 0;
        }
        current_cycle = 0;
    }
    
    bool isMeasuringInProgress() const { return isMeasuring; }
    
private:
    // Логирование для стадии измерений (MEASURING)
    void logMeasurement(int index) {
        updateCurrentVoltge();
        Serial.print("[MEASURING] (");
        Serial.print(index + 1);
        Serial.print("/");
        Serial.print(target_cycles);
        Serial.print(") V = ");
        Serial.print(current_voltage);
        Serial.print(" V | I = ");
        Serial.print(current_buffer[index]);
        Serial.println(" mA");
    }

    // Логирование для стадий работы терморегулятора, кроме стадии измерений (MEASURING)
    void logStatus() {
        switch (current_state) {
            case HOLDING:  Serial.print("[HOLDING]"); break;
            case SWITCHING: Serial.print("[STAGE SWITCHING]"); break;
        }
        if (current_state == HOLDING) {
            unsigned long now = millis();
            float elapsed_hold = (now - hold_start_time) / 1000.0; // s
            float completion_degree = elapsed_hold / hold_time * 100; // %
            required_voltage = (float)required_level / MAX_VALUE * Vref;
            updateCurrentLevelAndVoltge();
            Serial.print(" REQUIRED LEVEL = "); Serial.print(required_level); 
            Serial.print(" | REQUIRED V = "); Serial.print(required_voltage, 1); Serial.print(" V");
            Serial.print(" | CURRENT LEVEL = "); Serial.print(current_level); 
            Serial.print(" | CURRENT V = "); Serial.print(current_voltage, 1); Serial.println(" V");
            Serial.print("Comletion degree of the holding stage = "); Serial.print(completion_degree, 1); Serial.println(" %");
        }
    }

    int getCurrentLevel() const { return current_level;}
    void updateCurrentLevel() {
        current_level = analogRead(output_DAC_pin);
    }
    float getCurrentVoltage() const { return current_voltage;}
    void updateCurrentVoltge() {
        current_voltage = (float)current_level / MAX_VALUE * Vref;
    }
    void updateCurrentLevelAndVoltge() {
        updateCurrentLevel();
        updateCurrentVoltge();
    }
};
