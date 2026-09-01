#pragma once

#include "ina219_driver.h"

#define CURRENT_SENSORS_NUM 3

//A namespace in C++ is essentially a good and simple alternative to a static class. A perfect fit for singleton pattern.
//Declarations in this header will be included in other source files and therefore will be visible outside of current_sensors.cpp.
//Only "public" API goes here.
namespace current_sensors
{
    //An array of these descriptors can be configurable and can be stored in EEPROM
    //Therefore it contains only external data that the code itself can't know (address jumper positions, real shunt resistor values)
    //All dynamic values (measured current) and other implementation details are hidden inside the private part of this namespace
    struct descriptor_t
    {
        uint8_t address;
        float shunt_resistance_ohms;
    };

    //This receives the descriptors, since they contain information that is required for initialization.
    void init(const descriptor_t descriptors[CURRENT_SENSORS_NUM]); //Btw, this is a declaration.
    
    void read_all();

    float get_current_amps(uint8_t sensor_index); //Index spans from 0 to CURRENT_SENSORS_NUM-1


    void init_by_address(const uint8_t address, const float shunt_resistance_ohms, bool& initialization_ok);
    float get_current_amps_by_address(const uint8_t address, const float shunt_resistance_ohms, bool& initialization_ok);
    //void get_current_amps_by_address(const uint8_t address);
} // namespace current_sensors
