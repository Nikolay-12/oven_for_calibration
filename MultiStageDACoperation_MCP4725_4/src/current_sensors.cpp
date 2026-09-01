#include "current_sensors.h"

#include "ina219_driver.h"
#include <Wire.h>

#define MY_INA219_CAL_MAGIC 409.6 /*4096 Pasha: 33554.4 Divide by ohms */
#define MY_INA219_CURRENT_LSB 1.0E-4  // 1.0E-4 Pasha: 1.2207E-6   CALC_VALUE = trunc(0.04096/CURRENT_LSB*Rshunt)
uint16_t USED_CONFIG = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; 
/*
uint16_t USED_CONFIG = INA219_CONFIG_BVOLTAGERANGE_32V |
                  INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; //32V_1A configuration -- CurrentLSB = 0.00004 -- calValue = 10240
uint16_t USED_CONFIG = INA219_CONFIG_BVOLTAGERANGE_16V |
                  INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_BADCRES_12BIT |
                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; //16V_0.4A configuration  -- CurrentLSB = 0.00005 -- calValue = 8192
*/

//Anything declared (but not defined!) here will not be visible in other source files. Therefore instance_t is "private".
//Remember that the first definitions without a preceding declaration will be considered as both.
//So the forward-declaration rule is not violated.
namespace current_sensors
{
    struct instance_t
    {
        //We will not store the descriptor here to avoid memory duplication. Let higher-level code decide where to store the descriptors.
        //Therefore this is just a pointer.
        const descriptor_t* d; //Const-correctness: the code that uses instance_t should never modify the descriptor values it receives from higher-level code
        bool initialization_ok;
        float current_amps;
    };
    instance_t instances[CURRENT_SENSORS_NUM];

    void init(const descriptor_t descriptors[CURRENT_SENSORS_NUM]) //And this is a definition. Definition includes the body {}.
    {
        //First, initialize the underlying Arduino I2C bus driver: TwoWire
        //(the default instance of TwoWire class is called 'Wire' and is allocated to the first hardware I2C peripheral by Arduino framework during startup,
        //and on atmega328 the first I2C peripheral happens to be the only one)
        Wire.begin();

        //Now initialize the I2C devices
        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
        {
            instance_t& inst = instances[i]; //This is pass-by-reference, so any modifications of inst below will affect instances[...]

            //Initialize instance structure for this sensor
            inst.d = &(descriptors[i]); //Remember the address of the descriptor storage (for this module) we get from some higher-level code
            inst.initialization_ok = false; //By default assume worst

            //Initialize INA219s
            if (INA219::isConnected(inst.d->address)) //Success: the device is present and ready to accept instructions
            {
                INA219::reset(inst.d->address);
                INA219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_resistance_ohms)), 
                    MY_INA219_CURRENT_LSB);
                INA219::setConfig(inst.d->address, USED_CONFIG); // set configuration
                inst.initialization_ok = true;
            }
        }
    }

    void read_all()
    {
        //This is a for-range loop, an example of modern C++'s syntactic sugar, it is functionally equivallent to:
        //for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) { instance_t& inst = instances[i]; ... }
        for (auto &&inst : instances)
        {
            if (!inst.initialization_ok) continue; //Skip devices that failed to answer during initialization. This is just an example, you might want to do something with them further.
            INA219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_resistance_ohms)), 
                    MY_INA219_CURRENT_LSB);
            inst.current_amps = INA219::readCurrent(inst.d->address);
        }
    }

    float get_current_amps(uint8_t sensor_index)
    {
        if (sensor_index >= CURRENT_SENSORS_NUM) return NAN; //So-called "guard-clause": it is placed in the beginning of the functions
        //to return from the function early in case something is wrong. Here it protects the rest of the code from an out-of-bounds memory read.
        return instances[sensor_index].current_amps;
    }


    void init_by_address(const uint8_t addr, const float shunt_resistance_ohms, bool& initialization_ok) {
        if (INA219::isConnected(addr)) //Success: the device is present and ready to accept instructions
        {
            INA219::reset(addr);
            INA219::setCalibration(addr, 
                static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / shunt_resistance_ohms)), 
                MY_INA219_CURRENT_LSB);
            INA219::setConfig(addr, USED_CONFIG); // set configuration
            initialization_ok = true;
            Serial.println("INA219 is initializied!");
        }
        else {
            Serial.println("INA219 isn't initializied!");
        }
    }

    float get_current_amps_by_address(const uint8_t addr, const float shunt_resistance_ohms, bool& initialization_ok) {
        if (!initialization_ok) {
            Serial.println("WARNING: INA219 not initialized!");
            return 0.0;
        }
        float current_mA = INA219::readCurrent(addr);
        return current_mA;  
    }
} // namespace current_sensors
