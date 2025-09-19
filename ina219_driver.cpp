#include "ina219_driver.h"

#include <Wire.h>

namespace ina219
{
    float ina219_current_lsb;

    /*
     * @brief:		Read a register from the INA219 sensor.
     * @param:		register address in hexadecimal
     * @retval:		16 bit unsigned integer that represents the register's contents.
     */
    uint16_t Read16(uint8_t addr, uint8_t Register)
    {
        uint8_t Value[2];
        Wire.beginTransmission(addr);
        Wire.write(Register);
        if (Wire.endTransmission() != 0) return 0;
        if (Wire.requestFrom(addr, static_cast<uint8_t>(2)) != 2) return 0;
        Value[0] = Wire.read(); //Responses are 16-bit wide according to the datasheet
        Value[1] = Wire.read();

        return ((Value[0] << 8) | Value[1]);
    }

    /*
     * @brief:		Write to a register on the INA219 sensor.
     * @param:		Register address in hexadecimal
     * @param:		16 bit integer in hexadecimal that is the value you want to write to the register.
     */
    uint8_t Write16(uint8_t addr, uint8_t Register, uint16_t Value)
    {
        Wire.beginTransmission(addr);
        Wire.write(Register);
        Wire.write(static_cast<uint8_t>((Value >> 8) & 0xFF));
        Wire.write(static_cast<uint8_t>(Value & 0xFF));
        return Wire.endTransmission(); 
    }

    /*
     *  @brief:	  	Gets the raw current value (16-bit signed integer, so +-32767)
     *  @retval:	The raw current reading
     */
    int16_t readCurrent_raw(uint8_t addr)
    {
        int16_t result = Read16(addr, INA219_REG_CURRENT);
        return (result);
    }

    /*
     * @brief:  	Gets the current value in mA, taking into account the config settings and current LSB
     * @return: 	The current reading convereted to amps
     */
    float readCurrent(uint8_t addr)
    {
        int16_t result = readCurrent_raw(addr);

        return (result * ina219_current_lsb);
    }

    /*
     * @brief: 		This function will read the shunt voltage level.
     * @retval:		Returns voltage level in mili-volts. This value represents the difference
     * 				between the voltage of the power supply and the bus voltage after the shunt
     * 				resistor.
     */
    float readShuntVolage(uint8_t addr)
    {
        int16_t result = Read16(addr, INA219_REG_SHUNTVOLTAGE);

        return (result * 0.01); // These are the subtleties of the module's operation
    }

    uint8_t reset(uint8_t addr)
    {
        return Write16(addr, INA219_REG_CONFIG, INA219_CONFIG_RESET);
    }

    void setCalibration(uint8_t addr, uint16_t CalibrationData, float current_lsb)
    {
        ina219_current_lsb = current_lsb;
        Write16(addr, INA219_REG_CALIBRATION, CalibrationData);
    }

    uint16_t getConfig(uint8_t addr)
    {
        uint16_t result = Read16(addr, INA219_REG_CONFIG);
        return result;
    }

    void setConfig(uint8_t addr, uint16_t Config)
    {
        Write16(addr, INA219_REG_CONFIG, Config);
    }

    void setPowerMode(uint8_t addr, uint8_t Mode)
    {
        uint16_t config = getConfig(addr);

        switch (Mode)
        {
        case INA219_CONFIG_MODE_POWERDOWN:
            config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_POWERDOWN & INA219_CONFIG_MODE_MASK);
            setConfig(addr, config);
            break;

        case INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED:
            config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED & INA219_CONFIG_MODE_MASK);
            setConfig(addr, config);
            break;

        case INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS:
            config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS & INA219_CONFIG_MODE_MASK);
            setConfig(addr, config);
            break;

        case INA219_CONFIG_MODE_ADCOFF:
            config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_ADCOFF & INA219_CONFIG_MODE_MASK);
            setConfig(addr, config);
            break;
        }
    }

    bool isConnected(uint8_t addr)
    {
        if ((addr < 0x40) || (addr > 0x4F)) return false;
        Wire.beginTransmission(addr);
        return (Wire.endTransmission() == 0);
    }
}
