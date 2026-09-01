#include "ina219_driver.h"

#include <Wire.h>

//This driver is implemented as just a namespace, not as a class, since OOP is more of a curse here than a blessing.
//We want this code to take device addresses and data as function arguments and perform data exchange by calling
//I2C driver APIs (Wire). No more no less. The exact address, calibration and other values are not of its concern.
//So no objects here. Only procedures or functions. This is a different programming paradigm altogether (compared to OOP).
namespace INA219
{
    float ina219_current_lsb;

    /*
     * @brief:		Read a register from the IN219 sensor.
     * @param:		register address in hexadecimal
     * @retval:		16 bit unsigned integer that represents the register's contents.
     */
    uint16_t Read16(uint8_t addr, uint8_t Register)
    {
        uint8_t Value[2];
        
        //Outgoing bytes are not actually sent until endTransmission() is called,
        //instead they are first buffered in a circular (FIFO) buffer hidden inside the TwoWire library.
        //What for? No idea. On STM32, for example, the developers of the HAL allow you to do the buffering yourself if needed,
        //instead of forcing an intermediate buffer on you even when you only need to send a single byte.
        //See, this is the problem with Arduino. Too many assumptions and too many attempts to oversimplify inherently complex tasks,
        //leading to lack of understanding how things actually work in favor of understanding how someone thought they should work.
        Wire.beginTransmission(addr); //Begin buffering bytes to be sent (spoiler: there will be only one)
        Wire.write(Register); //We send just the address of the register we want to read from
        if (Wire.endTransmission() != 0) return 0; //Finish buffering and actually send the byte.
        if (Wire.requestFrom(addr, static_cast<uint8_t>(2)) != 2) return 0; //Now we can read back what the slave device has to say.
        Value[0] = Wire.read(); //Responses are 16-bit wide according to the datasheet, so:
        Value[1] = Wire.read();

        return ((Value[0] << 8) | Value[1]); //Compose a uint16_t from 2 bytes, taking endianess into account.
    }

    /*
     * @brief:		Write to a register on the IN219 sensor.
     * @param:		Register address in hexadecimal
     * @param:		16 bit integer in hexadecimal that is the value you want to write to the register.
     */
    uint8_t Write16(uint8_t addr, uint8_t Register, uint16_t Value)
    {
        //See my rant about this software FIFO stuff in Read16(). Here we go again, although I am perfectly capable of buffering 3 bytes myself.
        Wire.beginTransmission(addr);
        Wire.write(Register);
        Wire.write(static_cast<uint8_t>((Value >> 8) & 0xFF));
        Wire.write(static_cast<uint8_t>(Value & 0xFF)); // lower byte
        return Wire.endTransmission(); //Should return 0 == OK
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
     * @brief:  	Gets the current value in mA, taking into account the
     *          	config settings and current LSB
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

        return (result * 0.01); // ????
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
} // namespace ina219
