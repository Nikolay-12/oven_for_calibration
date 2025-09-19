#include "current_sensors.h"

#include "ina219_driver.h"
#include <Wire.h>

// for 32V_2A configuration
#define MY_INA219_CAL_MAGIC 409.6 /* Divide by ohms */
#define MY_INA219_CURRENT_LSB 1.0E-4 // CALC_VALUE = trunc(0.04096/CURRENT_LSB*Rshunt)
uint16_t USED_CONFIG = INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; //32V_2A configuration -- CurrentLSB = 0.0001 -- calValue = 4096
/*
uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; //32V_1A configuration -- CurrentLSB = 0.00004 -- calValue = 10240
uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
                  INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; //16V_0.4A configuration  -- CurrentLSB = 0.00005 -- calValue = 8192
*/

namespace current_sensors
{
    struct instance_t
    {
        const descriptor_t* d; 
        bool initialization_ok;
        float current_amps;
    };
    instance_t instances[CURRENT_SENSORS_NUM];

    void init(const descriptor_t descriptors[CURRENT_SENSORS_NUM])
    {
        Wire.begin();

        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
        {
            instance_t& inst = instances[i];

            //Initialize instance structure for each sensor
            inst.d = &(descriptors[i]); 
            inst.initialization_ok = false; //By default assume worst

            //Initialize INA219s
            if (ina219::isConnected(inst.d->address))
            {
                ina219::reset(inst.d->address);
                ina219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_resistance_ohms)), 
                    MY_INA219_CURRENT_LSB);
                ina219::setConfig(inst.d->address, USED_CONFIG) // set configuration
                inst.initialization_ok = true;
            }
        }
    }

    void read_all()
    {
        for (auto &&inst : instances)
        {
            if (!inst.initialization_ok) continue;
            ina219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_resistance_ohms)), 
                    MY_INA219_CURRENT_LSB);
            inst.current_amps = ina219::readCurrent(inst.d->address);
        }
    }

    float get_current_amps(uint8_t sensor_index)
    {
        if (sensor_index >= CURRENT_SENSORS_NUM) return NAN;
        return instances[sensor_index].current_amps;
    }
}
