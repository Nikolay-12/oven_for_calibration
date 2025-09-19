#include "74HC595_register_and_MAX6675.h"
#include <SPI.h>

namespace 74HC595
{
  void setting_up_reg_pins()
  {
    pinMode(HC595_DS_PIN, OUTPUT);
    pinMode(HC595_STcp_PIN, OUTPUT);
    pinMode(HC595_SHcp_PIN, OUTPUT);
    digitalWrite(HC595_DS_PIN, HIGH);
    digitalWrite(HC595_STcp_PIN, HIGH);
    digitalWrite(HC595_SHcp_PIN, LOW);
  }

  void set_bit_on_DS_pin(bit)
  {
    digitalWrite(HC595_DS_PIN, bit);
    delayMicroseconds(10);
  }
  void shift_bit_in_reg()
  {
    digitalWrite(HC595_SHcp_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HC595_SHcp_PIN, LOW);
    delayMicroseconds(10);
  }
  void save_data_in_reg()
  {
    digitalWrite(HC595_STcp_PIN, LOW);
    delayMicroseconds(10);
    digitalWrite(HC595_STcp_PIN, HIGH);
    delayMicroseconds(10);
  }

  void set_11111111_in_reg()
  {
    for (int i = 1; i <= 8; i++) {
      void set_bit_on_DS_pin(HIGH);
      void shift_bit_in_reg();
    }
    void save_data_in_reg();
  }
  void add_0_to_reg()
  {
    void set_bit_on_DS_pin(LOW);
    void shift_bit_in_reg();
    void set_bit_on_DS_pin(HIGH);
    void save_data_in_reg();
  }
  void shift_and_save_0_in_reg()
  {
    void shift_bit_in_reg();
    void save_data_in_reg();
  }
}

namespace MAX6675
{
  void setting_pin_values_for_SPI()
  {
    pinMode(NANO_CLK_PIN, OUTPUT);
    digitalWrite(NANO_CLK_PIN, HIGH);
  }

  double readCelsius() 
  {
    uint16_t v;
    
    v = SPI.transfer(0x00);
    v <<= 8;
    v |= SPI.transfer(0x00);
    
    if (v & 0x4) {
        // uh oh, no thermocouple attached!
        return NAN; 
    }

    v >>= 3;
    return v*0.25;
  }
}
