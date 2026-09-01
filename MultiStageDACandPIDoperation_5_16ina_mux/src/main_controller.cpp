#include "main_controller.h"

namespace INA219 = INA219controller;

namespace MainController 
{
    bool state;
    Mode main_mode = Mode::IT_MODE;

    bool error_flag;

    void init() {
        Serial.println("Терморегулятор, ЦАП, INA219 и MOSFET для управления реле начинают совместную работу...");
        thermoRegulator::init();
        DAC_MCP4725::init();
        INA219::init();
        MOSFET::init();
        Serial.println("Устройства инициализированы. Выставлены настройки по умолчанию.");
        Serial.println("Реле находится в состоянии покоя. Выставлен режим измерения I(T).");
        Serial.println("Для изменения режима введите команду 'mainController:setMainMode:1;' ");
        Serial.println("Режим 1 -- измерение I(T)");
        Serial.println("Режим 2 -- измерение I(V)");
        state = true;
    }

    void inProgress() {
        if ((main_mode == Mode::IV_MODE) && (DAC_MCP4725::getState())) {
            DAC_MCP4725::inProgress(); // включение ЦАП
        }
        if ((main_mode == Mode::IT_MODE) && (thermoRegulator::getState())) {
            thermoRegulator::thermoregulation(); // включение терморегулятора
        }

        // Тестирование работы термопары
        if ((main_mode == Mode::IT_MODE) && (thermoRegulator::getReadingStatus())) {
            thermoRegulator::tempReading();
        }
        // Тестирование работы INA219
        if ((main_mode != Mode::IDLE) && INA219::getState() && (INA219::getMode() == 1)) { // чтение показаний INA219 N раз
            INA219::readCurrentSeveralTimes();
        }
        if ((main_mode != Mode::IDLE) && INA219::getState() && (INA219::getMode() == 2)) { // чтение показаний INA219 до прерывания
            INA219::currentReading();
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
                    MOSFET::changeState(0);
                    break;
                case Mode::IT_MODE:
                    Serial.println("Установлен режим измерения I(T)");
                    MOSFET::changeState(0);
                    break;
                case Mode::IV_MODE:
                    Serial.println("Установлен режим измерения I(V)");
                    MOSFET::changeState(1);
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
    bool canStartThermoreg() {
        if (!isMode(Mode::IT_MODE)) {
            Serial.println("Ошибка: режим I(T) не установлен! Сначала введите команду: mainController:setMainMode:1;");
            return false;
        }
        return true;
    }
    bool canStartDAC() {
        if (!isMode(Mode::IV_MODE)) {
            Serial.println("Ошибка: режим I(V) не установлен! Сначала введите команду: mainController:setMainMode:2;");
            return false;
        }
        return true;
    }
};