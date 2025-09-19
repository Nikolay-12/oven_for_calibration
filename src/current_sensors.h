#pragma once

#include "ina219_driver.h"

#define CURRENT_SENSORS_NUM 3

namespace current_sensors
{
    struct descriptor_t
    {
        uint8_t address;
        float shunt_resistance_ohms;
    };

    void init(const descriptor_t descriptors[CURRENT_SENSORS_NUM]); 
    
    void read_all();

    float get_current_amps(uint8_t sensor_index); //Index spans from 0 to CURRENT_SENSORS_NUM-1
} 
