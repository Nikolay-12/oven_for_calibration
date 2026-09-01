#pragma once

#include "project_config.h"


namespace INA219controller
{
    void init(); //  Команда для инициализации шины I²С и сенсоров
    void updateMeasurementSettings(const int cycles, const float periodization); //  Команда для установления настроек считывания с сенсоров
    
    //  Команды для чтения данных / их вывода в Serial порт
    float readINAcurrent(uint8_t address, float shunt_resistance_ohms, bool ina_initialization_ok); // // Считывание одного тока по адресу, шунту и инициализации
    void read_all_currents(); // Считывание всех токов с полным описанием (адрес, канал, значение, размерность)
    void readCurrents(); // Считывание всех токов в формате "Current #N: 0.32 mA"
    void readCurrentArray(); // Считывание всех токов в формате "0.31 0.1 0.4 1.5 ..."
    void readSingleSensor(size_t index); // Считывание одного тока с полным описанием (адрес, канал, значение, размерность)
    void readChannel(uint8_t channel); // Считывание токов с одного канала с полным описанием (адрес, канал, значение, размерность)

    //  Команды для запуска и остановки чтения данных / их вывода в Serial порт
    void startEndlessReading();
    void currentReading();
    void startReadingSeveralTimes();
    void readCurrentSeveralTimes();
    void on();
    void off();

    //  Команда для получения состояния и режима измерений
    bool getState();
    uint8_t getMode();
};