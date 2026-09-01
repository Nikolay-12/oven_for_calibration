#pragma once
#include <Arduino.h>
#include <Command_Parser_2.h>
#include "MultiStagePIDController.h"
#include "ThermalModel.h"
#include "PIDregulator.h"

// Настройка параметров для ПИД-регулятора
const float DEFAULT_KP = 10.0;
const float DEFAULT_KI = 0.3;
const float DEFAULT_KD = 0.8;
const float DEFAULT_DT = 0.1;  // Шаг дискретизации 100 мс
const float DEFAULT_stabilization_time = 5.0; // Время, которое необходимо для подтверждения стабилизации температуры (с)
const float DEFAULT_holding_time = 10.0; // Время удержания данной температуры (в это время идет сбор всех данных с датчиков) (с)
const float DEFAULT_temp_tolarance = 1.0; // Температурная погрешность
const float DEFAULT_max_stab_time = 30.0; // максимальное время на одной стадии
const float DEFAULT_TEMP_STEP = 10;
const float DEFAULT_INITIAL_TEMP = 30;
const float DEFAULT_FINAL_TEMP = 100;

extern MultiStagePIDController controller;
extern ThermalModel heater;
extern PIDController pid;

namespace thermoRegulator
{
    void clear();
    void init();
    void updateStages(const char* stages); // updating stages of the thermoregulator    int count, float* new_stages
    void updateStagesAuto(const float* STAGESparam);
    void resetStages();
    void updatePIDcoef(const float* PIDparam); // updating PID settings of the thermoregulator
    void updateTimeSettings(const float* TIMEparam); 
    // updating time settings of the thermoregulator
    void setTemp(const float* new_tempPoint);
    void thermoregOn(); // turn on the thermoregulator
    void thermoregOff(); // turn off the thermoregulator
    void thermoregulation(); // the main function of thermoregulator during the themperature regulation process
    bool getState();
    void getData();
}

