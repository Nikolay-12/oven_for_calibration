#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Command_Parser_2.h>
#include "MultiStagePIDController.h"
#include "PIDregulator.h"
#include "ina219_driver.h"
#include "current_sensors.h"
#include "Measurement_Processing.h"
#include "74HC595_register.h"


// Настройка параметров для ПИД-регулятора
const float DEFAULT_KP = 10.0;
const float DEFAULT_KI = 0.3;
const float DEFAULT_KD = 0.8;
const float DEFAULT_DT = 0.1;  // Шаг дискретизации 100 мс
const float DEFAULT_stabilization_time = 5.0; // Время, которое необходимо для подтверждения стабилизации температуры (с)
const float DEFAULT_holding_time = 10.0; // Время удержания данной температуры (в это время идет сбор всех данных с датчиков) (с)
const float DEFAULT_temp_tolarance = 1.0; // Температурная погрешность
const float DEFAULT_max_stab_time = 30.0; // максимальное время на одной стадии
const float DEFAULT_TEMPERATURE_STEP = 10;
const float DEFAULT_TEMPERATURE_MIN = 30;
const float DEFAULT_TEMPERATURE_MAX = 100;

extern MultiStagePIDController controller;
extern PIDController pid;
extern ShiftRegister74HC595 reg;


namespace thermoRegulator
{
    void clear();
    void init();
    void updateStages(const char* stages); // updating stages of the thermoregulator    int count, float* new_stages
    void updateTempSettings(const float* STAGESparam);
    void resetStages();
    void updatePIDcoef(const float* PIDparam); // updating PID settings of the thermoregulator
    void updateTimeSettings(const float* TIMEparam); 
    // updating time settings of the thermoregulator
    void updateMeasurementSettings(const int cycles_num, const float period_s);
    void setTempPoint(const float new_tempPoint);
    void changeMode(const int mode);

    void on(); // turn on the thermoregulator
    void off(); // turn off the thermoregulator
    void thermoregulation(); // the main function of thermoregulator during the themperature regulation process
    
    void startTempReading();
    void stopTempReading();
    void tempReading();
    bool getReadingStatus();
    
    bool getState();
    uint8_t getMode();
    void getData();

    //  ФУНКЦИЯ ДЛЯ НЕПОСРЕДСТВЕННОЙ РАБОТЫ С ТЕРМОРЕГУЛЯТОРОМ
    float readTemperature();
    void setHeaterOutput(float percent);

    void readAllTempAndReturnAverage();
    void readAllTempAndReturnThem();
    void stopReadingAllTemps();
};

namespace INA219_dop
{
    void init();
    void updateMeasurementSettings(const int cycles, const float periodization); 
    float readINAcurrent();
    void currentReading();
    void INA219on();
    void INA219off();
    void startEndlessReading();
    void currentReading();
    void startReadingSeveralTimes();
    void readCurrentSeveralTimes();

    bool getState();
    bool getMode();
};

void initI2C(uint8_t SDA_PIN, uint8_t SCL_PIN);
