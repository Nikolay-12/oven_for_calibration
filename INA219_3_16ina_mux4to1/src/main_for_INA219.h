#pragma once

#include "project_config.h"
#include "ina219_driver.h"
#include "current_sensors.h"

namespace INA219_dop
{
    void init();
    void updateMeasurementSettings(const int cycles, const float periodization);
    
    void read_all_currents(); // считывание всех токов с полным описанием (адрес, канал, значение, размерность)
    void readCurrents(); // считывание всех токовВ формате "Current #N: 0.32 mA"
    void readCurrentArray(); // считывание всех токов в формате "0.31 0.1 0.4 1.5 ..."
    void readSingleSensor(size_t index); // считывание одного тока с полным описанием (адрес, канал, значение, размерность)
    void readChannel(uint8_t channel); // считывание токов с одного канала с полным описанием (адрес, канал, значение, размерность)

    void on();
    void off();

    void startEndlessReading();
    void currentReading();
    void startReadingSeveralTimes();
    void readCurrentSeveralTimes();

    bool getState();
    uint8_t getMode();
};

