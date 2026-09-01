#include "current_sensors.h"
#include "MUX4to1.h"
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
        uint8_t mux_channel;
    };
    instance_t instances[CURRENT_SENSORS_NUM];
    static uint8_t last_channel = 0xFF; // Для отслеживания переключений

    void init(const descriptor_t descriptors[CURRENT_SENSORS_NUM])
    {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); // for ESP32
        MuxController::init(); // Инициализация мультиплексора
        
        MuxController::selectChannel(0); // Переключаемся на канал 0
        last_channel = 0; 

        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
        {
            instance_t& inst = instances[i];

            //Initialize instance structure for each sensor
            inst.d = &(descriptors[i]); 
            inst.current_amps = 0.0f;
            inst.mux_channel = descriptors[i].MUX_channel;
            inst.initialization_ok = false; //By default assume worst

                        
            if (descriptors[i].MUX_channel != last_channel) { // Проверяем, нужно ли переключать мультиплексор
                MuxController::selectChannel(descriptors[i].MUX_channel);
                last_channel = descriptors[i].MUX_channel;
                delayMicroseconds(100); // Небольшая задержка для стабилизации
            }

            //Initialize INA219s
            if (ina219::isConnected(inst.d->address))
            {
                ina219::reset(inst.d->address);
                ina219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_R1_ohms)), 
                    MY_INA219_CURRENT_LSB);
                ina219::setAveraging(inst.d->address, 7); // set averaging
                ina219::setConfig(inst.d->address, USED_CONFIG); // set configuration
                inst.initialization_ok = true;

                // Отладочная информация
                Serial.print("Датчик ");
                Serial.print(i);
                Serial.print(" (адрес 0x");
                Serial.print(inst.d->address, HEX);
                Serial.print(", канал ");
                Serial.print(inst.d->MUX_channel);
                Serial.println(") инициализирован");
            } else {
                Serial.print("Ошибка инициализации датчика ");
                Serial.print(i);
                Serial.print(" (адрес 0x");
                Serial.print(inst.d->address, HEX);
                Serial.print(", канал ");
                Serial.print(inst.d->MUX_channel);
                Serial.println(")");
            }
        }

        // Возвращаемся на канал 0
        MuxController::selectChannel(0);
        last_channel = 0;
        
        Serial.print("Инициализировано датчиков: ");
        Serial.println(countInitialized());
    }

    void read_all_on_one_channel()
    {
        for (auto &&inst : instances)
        {
            if (!inst.initialization_ok) continue;
            ina219::setCalibration(inst.d->address, 
                    static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_R1_ohms)), 
                    MY_INA219_CURRENT_LSB);
            inst.current_amps = ina219::readCurrent(inst.d->address);
        }
    }
    void read_all_16() {
        for (auto &&inst : instances)
        {
            if (!inst.initialization_ok) return;

            if (inst.d->MUX_channel != last_channel) { // Проверяем, нужно ли переключать мультиплексор
                MuxController::selectChannel(inst.d->MUX_channel);
                last_channel = inst.d->MUX_channel;
                delayMicroseconds(100);
            }
        
            ina219::setCalibration(
                inst.d->address,
                static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / inst.d->shunt_R1_ohms)),
                MY_INA219_CURRENT_LSB
            );
            inst.current_amps = ina219::readCurrent(inst.d->address);
        }
    }
    void read_single(size_t sensor_index) {
        if (sensor_index >= CURRENT_SENSORS_NUM || !instances[sensor_index].initialization_ok) return;
        
        if (instances[sensor_index].d->MUX_channel != last_channel) {
            MuxController::selectChannel(instances[sensor_index].d->MUX_channel);
            last_channel = instances[sensor_index].d->MUX_channel;
            delayMicroseconds(100);
        }
        
        ina219::setCalibration(
            instances[sensor_index].d->address,
            static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / instances[sensor_index].d->shunt_R1_ohms)),
            MY_INA219_CURRENT_LSB
        );
        instances[sensor_index].current_amps = ina219::readCurrent(instances[sensor_index].d->address);
    }

    float get_current_amps(uint8_t sensor_index)
    {
        if (sensor_index >= CURRENT_SENSORS_NUM) return NAN;
        return instances[sensor_index].current_amps;
    }
    void get_all_currents(float* currents) {
        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
            currents[i] = instances[i].current_amps;
        }
    }

    bool is_initialized(size_t index) {
        if (index >= CURRENT_SENSORS_NUM) return false;
        return instances[index].initialization_ok;
    }


    void init_by_address(const uint8_t addr, const float shunt_resistance_ohms, bool& initialization_ok) {
        if (ina219::isConnected(addr)) //Success: the device is present and ready to accept instructions
        {
            ina219::reset(addr);
            ina219::setCalibration(addr, 
                static_cast<uint16_t>(roundf(MY_INA219_CAL_MAGIC / shunt_resistance_ohms)), 
                MY_INA219_CURRENT_LSB);
            ina219::setConfig(addr, USED_CONFIG); // set configuration
            initialization_ok = true;
            Serial.println("Датчик тока INA219 инициализирован.");
        }
        else {
            Serial.println("Датчик тока INA219 не инициализирован!");
        }
    }

    float get_current_amps_by_address(const uint8_t addr, const float shunt_resistance_ohms, bool& initialization_ok) {
        if (!initialization_ok) {
            Serial.println("Предупреждение: Датчик тока INA219 не инициализирован!");
            return 0.0;
        }
        float current_mA = ina219::readCurrent(addr);
        return current_mA;  
    }


    // Вспомогательная функция для подсчета инициализированных датчиков
    uint8_t countInitialized() {
        uint8_t count = 0;
        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
            if (instances[i].initialization_ok) count++;
        }
        return count;
    }
} 
