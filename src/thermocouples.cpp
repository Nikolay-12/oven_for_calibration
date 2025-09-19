#include "thermocouples.h"
#include "74HC595_register_and_MAX6675.h"
#include <SPI.h>

namespace thermocouples
{
  struct instance_t
  {
    float temp_celsius;
  };
  instance_t instances[THERMOCOUPLES_NUM];

  void init() 
  {
    74HC595::setting_up_reg_pins();
    MAX6675::setting_pin_values_for_SPI();
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    74HC595::set_11111111_in_reg();
  }

  void read_all_temp_once() 
  {
    74HC595::add_0_to_reg();
    instances[0].temp_celsius = MAX6675::readCelsius();
    for (int i = 1; i <= THERMOCOUPLES_NUM - 1; i++) {
      74HC595::shift_and_save_0_in_reg();
      instances[i].temp_celsius = MAX6675::readCelsius();
    }
  }
  void read_all_temp() 
  {
    void read_all_temp_once();
  }

  void read_temp_from_one_thermocouple_once(thermocouple_index) 
  {
    74HC595::set_bit_on_DS_pin(LOW);
    74HC595::shift_bit_in_reg();
    74HC595::set_bit_on_DS_pin(HIGH);
    if (thermocouple_index - 1 == 0)
    {
      74HC595::save_data_in_reg();
      instances[0].temp_celsius = MAX6675::readCelsius();
    }
    else
    {
      for (int i = 1; i <= thermocouple_index - 1; i++) {
        74HC595::shift_bit_in_reg();
        74HC595::save_data_in_reg();
        instances[i].temp_celsius = MAX6675::readCelsius();
    }
  }
  void read_temp_from_one_thermocouple(thermocouple_index) 
  {
    void read_temp_from_one_thermocouple_once(thermocouple_index);
  }

  void stop_reading_temp()
  {
    74HC595::set_11111111_in_reg();
  }

  float get_temp_celsius(thermocouple_index)
  {  
    if (thermocouple_index >= THERMOCOUPLES_NUM) return NAN;
    return instances[i].temp_celsius;
  }
}
