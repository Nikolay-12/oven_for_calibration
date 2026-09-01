#pragma once
#include "project_config.h"
#include "ina219_driver.h"

namespace current_sensors
{
    using descriptor_t = INA219_descriptor;

    void init(const descriptor_t CURRENT_SENSOR_DESCRIPTORS[CURRENT_SENSORS_NUM]); 
    
    void read_all_on_one_channel();
    void read_all_16();
    void read_single(size_t index);

    float get_current_amps(uint8_t sensor_index); //Index spans from 0 to CURRENT_SENSORS_NUM-1
    void get_all_currents(float* currents);

    bool is_initialized(size_t index);

    void init_by_address(const uint8_t address, const float shunt_resistance_ohms, bool& initialization_ok);
    float get_current_amps_by_address(const uint8_t address, const float shunt_resistance_ohms, bool& initialization_ok);
    //void get_current_amps_by_address(const uint8_t address);

    void set_calibration(bool mode);
    uint8_t countInitialized();
} 
