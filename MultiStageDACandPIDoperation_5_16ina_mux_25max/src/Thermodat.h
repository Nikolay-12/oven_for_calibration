#pragma once
#include <GyverMAX6675.h>
#include "project_config.h"
#include "MultiStagePIDController.h"
#include "PIDregulator.h"
#include "Measurement_Processing.h"
#include "74HC595_register.h"

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
    void updateTimeTolSettings(const float* TIMEparam); 
    // updating time settings of the thermoregulator
    void updateMeasurementSettings(const int cycles_num, const float period_s);
    void setTempPoint(const float new_tempPoint);
    void changeMode(const int mode);

    void on(); // turn on the thermoregulator
    void off(); // turn off the thermoregulator
    void thermoregulation(); // the main function of thermoregulator during the themperature regulation process
    
    bool getState();
    uint8_t getMode();
    void getData();

    //  ФУНКЦИЯ ДЛЯ НЕПОСРЕДСТВЕННОЙ РАБОТЫ С ТЕРМОРЕГУЛЯТОРОМ
    float readTemperature();
    void setHeaterOutput(float percent);

    void startTempReading();
    void stopTempReading();
    void tempReading();
    bool getReadingStatus();
    void readAvrTemp();
    void readAllTemps();
    void readAllTempAndReturnAverage();
    void readAllTempAndReturnThem();
};
