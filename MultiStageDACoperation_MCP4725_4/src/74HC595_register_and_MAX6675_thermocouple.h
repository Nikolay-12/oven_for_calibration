#pragma once
#include <stdint.h>

#define NANO_CLK_PIN 13 // Пин SCK термопар
#define NANO_MISO_PIN 12 // ПИН MISO термопар
#define HC595_DS_PIN 2 // Пин данных для сдвиговых регистов ("путешествующий ноль")
#define HC595_STcp_PIN 3 // Пин для защелкивания выходов сдвиговых регистов
#define HC595_SHcp_PIN 4 // Пин тактовых импульсов для сдвиговых регистов

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
