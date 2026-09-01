#pragma once
#include "project_config.h"

namespace register_74HC595
{
  void setting_up_reg_pins();
  void set_0_on_DS_pin();
  void set_1_on_DS_pin();
  void shift_bits_in_reg();
  void save_data_in_reg();
  void set_11111111_in_reg();
  void add_0_to_reg();
  void shift_and_save_data_in_reg();
  void set_byte_in_reg(uint8_t value); // The value is allowed to be from 0 to 255
}

namespace thermocouple_MAX6675
{
  void setting_pin_values_for_SPI();
  double readCelsius();
}
