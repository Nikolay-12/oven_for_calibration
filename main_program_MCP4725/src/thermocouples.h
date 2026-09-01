#pragma once
#include "project_config.h"
#include "74HC595_register.h"
#include "MAX6675_thermocouple.h"


namespace thermocouples
{
    // Команда для инициализации термопар (+ сдвигового регистр)
    void init(ShiftRegister74HC595* reg);

    // Команды для считывания температур
    void read_all_temp_once(bool calibrate);
    //void read_all_temp(bool calibrate);
    void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index);
    //void read_temp_from_one_thermocouple(uint8_t thermocouple_index);
    void stop_reading_temp();
    
    // Команды для получения температуры с выбранной термопары и для получения усредненной температуры
    double get_temp_celsius(uint8_t thermocouple_index);
    double get_average_temp_celsius();
    // Команда для усреднения температуры по всем термопарам
    void average_temp_celsius(bool calibrate);
    // Команды для вывода в COM-порт температур с термопар
    void send_temp_point(uint8_t thermocouple_index, bool calibrate);
    void send_all_temp(bool calibrate);
    void send_temp_array(bool calibrate);
    void send_temp_array_and_avr(bool calibrate);

    // Функция для получения откалиброванной температуры
    float get_calibrated_temp(uint8_t thermocouple_index);
}

