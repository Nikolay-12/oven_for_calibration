#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "project_config.h"
#include "74HC595_register.h"
#include "MAX6675_thermocouple.h"


namespace thermocouples
{
    void init(ShiftRegister74HC595* reg);
    void read_all_temp_once();
    void read_all_temp();
    void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index);
    void read_temp_from_one_thermocouple(uint8_t thermocouple_index);
    void stop_reading_temp();
    double get_temp_celsius(uint8_t thermocouple_index);
    double get_average_temp_celsius();
    void average_temp_celsius();
    void send_temp_point(uint8_t thermocouple_index);
    void send_all_temp();
    void send_temp_array();
}

