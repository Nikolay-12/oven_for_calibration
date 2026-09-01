#include "main_controller.h"


namespace MainController 
{
    bool state;
    Mode main_mode = Mode::IT_MODE;

    bool error_flag;

    void init() {
        Serial.println("Measuring current with 16 INA219 sensors...");
        current_sensors::init(CURRENT_SENSOR_DESCRIPTORS);
        Serial.println("Initializing was completed. The system is ready to start.");
        state = true;
    }

    void inProgress() {
        // Тестирование работы INA219
        if ((main_mode != Mode::IDLE) && INA219_dop::getState() && (INA219_dop::getMode() == 1)) { // чтение показаний INA219 N раз
            INA219_dop::readCurrentSeveralTimes();
        }
        if ((main_mode != Mode::IDLE) && INA219_dop::getState() && (INA219_dop::getMode() == 2)) { // чтение показаний INA219 до прерывания
            INA219_dop::currentReading();
        }
    }
    void stop() {
        setState(false);
    }
    
    void setState(bool new_state) {
        state = new_state;
    }
    bool getState() { return state; }
    void setMainMode(int new_mode) {
        if ((new_mode == 0) || (new_mode == 1) || (new_mode == 2)) {
            main_mode = static_cast<Mode>(new_mode);
            switch (main_mode) {
                case Mode::IDLE:
                    Serial.println("Холостой режим установлен");
                    break;
                case Mode::IT_MODE:
                    Serial.println("Установлен режим измерения I(T)");
                    break;
                case Mode::IV_MODE:
                    Serial.println("Установлен режим измерения I(V)");
                    break;
            }
        }
        else {
            Serial.println("Нет такого режима! Установлен холостой режим.");
            main_mode = Mode::IDLE;
        }
        
    }
    uint8_t getMainMode() { return static_cast<uint8_t>(main_mode); }

    bool isMode(Mode expectedMode) {
         return getMainMode() == static_cast<uint8_t>(expectedMode);
    }
};