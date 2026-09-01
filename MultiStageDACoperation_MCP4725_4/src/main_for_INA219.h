#pragma once

#include <Wire.h>
#include <Command_Parser_2.h>
#include "project_config.h"
#include "ina219_driver.h"
#include "current_sensors.h"
#include "main_for_DAC_MCP4725.h"

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