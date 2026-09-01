#include "main_controller.h"

namespace INA219 = INA219controller;

namespace MainController 
{
    bool state;
    Mode main_mode = Mode::IT_MODE;

    bool error_flag;

    void init() {
        Serial.println("The thermostat, DAC, INA219 and MOSFET for relay control begin their joint work...");
        thermoRegulator::init();
        DAC_MCP4725::init();
        INA219::init();
        MOSFET::init();
        Serial.println("The devices have been initialized and the default settings have been set.");
        Serial.println("The relay is in the idle state. The I(T) measurement mode is set.");
        Serial.println("To change the mode, enter the command 'mainController:setMainMode:1;' ");
        Serial.println("Mode 1 -- I(T) measurement");
        Serial.println("Mode 2 -- I(V) measurement");
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
                    Serial.println("Idle mode is set");
                    MOSFET::changeState(0);
                    current_sensors::set_calibration(0);
                    Serial.println("By default, the relay control transistor is set to I(T) measurement mode.");
                    Serial.println("By default, INA219 sensors are calibrated for measurements in I(T) mode.");
                    break;
                case Mode::IT_MODE:
                    Serial.println("The I(T) measurement mode is set");
                    MOSFET::changeState(0);
                    current_sensors::set_calibration(1);
                    break;
                case Mode::IV_MODE:
                    Serial.println("The I(V) measurement mode is set");
                    MOSFET::changeState(1);
                    current_sensors::set_calibration(0);
                    break;
            }
        }
        else {
            Serial.println("There is no such mode! Idle mode is set.");
            main_mode = Mode::IDLE;
        }
        
    }
    uint8_t getMainMode() { return static_cast<uint8_t>(main_mode); }

    bool isMode(Mode expectedMode) {
         return getMainMode() == static_cast<uint8_t>(expectedMode);
    }
    bool canStartThermoreg() {
        if (!isMode(Mode::IT_MODE)) {
            Serial.println("Error: I(T) mode is not set! First enter the command: mainController:setMainMode:1;");
            return false;
        }
        return true;
    }
    bool canStartDAC() {
        if (!isMode(Mode::IV_MODE)) {
            Serial.println("Error: I(V) mode is not set! First enter the command: mainController:setMainMode:2;");
            return false;
        }
        return true;
    }
};