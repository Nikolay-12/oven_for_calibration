#pragma once

#include "project_config.h"
#include "MultiStageDAC.h"
#include "Measurement_Processing.h"


extern MultiStageDACcontroller DACcontroller;

namespace DAC_MCP4725
{
    void clear();
    void init();
    
    void updateStages(const char* stages); // updating stages of the DAC  int count, float* new_stages
    void updateStagesAuto(const float* STAGESparam); // need?
    void setVoltagePoint(const float new_voltPoint); // setting a specific voltage value for the DAC (подготовка)
    void resetStages();
    void updateSettings(const float* param); // updating the settings of the DAC
    void updateTimeSettings(const float new_hold_time);
    void updateMeasurementSettings(const int cycles_num, const float period_s);
    
    void changeMode(const int mode);
    void inProgress(); // the main function for the DAC to operate according to a given scenario
    void on(); // turn on the DAC
    void off(); // turn off the DAC

    int getLevel();
    void getData();
    bool getState();
    uint8_t getMode();

    //  ФУНКЦИЯ ДЛЯ НЕПОСРЕДСТВЕННОЙ РАБОТЫ С ЦАП
    //void setVoltage(float voltage); // конвертирует напряжение в необходимый уровень сигнала для ЦАП
    void setOutputLevel(size_t level, bool save_to_eeprom); // устанавливает уровень сигнала ЦАП
};