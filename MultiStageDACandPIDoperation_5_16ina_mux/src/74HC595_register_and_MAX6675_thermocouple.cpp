#include "74HC595_register_and_MAX6675_thermocouple.h"
#include <SPI.h>
#include <Arduino.h>

namespace register_74HC595
{
  void setting_up_reg_pins()
  {
    pinMode(HC595_DS_PIN, OUTPUT);
    pinMode(HC595_ST_PIN, OUTPUT);
    pinMode(HC595_SH_PIN, OUTPUT);
    digitalWrite(HC595_DS_PIN, HIGH);
    digitalWrite(HC595_ST_PIN, HIGH);
    digitalWrite(HC595_SH_PIN, LOW);
  }

  void set_1_on_DS_pin() //20 mсs
  {
    digitalWrite(HC595_DS_PIN, HIGH);
    delayMicroseconds(20);
  }
  void set_0_on_DS_pin() //20 mcs
  {
    digitalWrite(HC595_DS_PIN, LOW);
    delayMicroseconds(20);
  }
  void shift_bits_in_reg() //20 mсs
  {
    digitalWrite(HC595_SH_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HC595_SH_PIN, LOW);
    delayMicroseconds(10);
  }
  void save_data_in_reg() //20 mсs
  {
    digitalWrite(HC595_ST_PIN, LOW);
    delayMicroseconds(10);
    digitalWrite(HC595_ST_PIN, HIGH);
    delayMicroseconds(10);
  }
  void add_0_to_reg() //80 mсs
  {
    set_0_on_DS_pin();
    shift_bits_in_reg();
    set_1_on_DS_pin();
    save_data_in_reg();
  }
  void shift_and_save_data_in_reg() //40 mсs
  {
    shift_bits_in_reg();
    save_data_in_reg();
  }

  void set_11111111_in_reg() //200 mсs
  {
    set_1_on_DS_pin();
    for (size_t i = 1; i <= 8; i++) {
      shift_bits_in_reg();
    }
    save_data_in_reg();
  }
  void set_byte_in_reg(uint8_t value) // The value is allowed to be from 0 to 255
  { 
    digitalWrite(HC595_ST_PIN, LOW);  // Turn off the output 
    shiftOut(HC595_DS_PIN, HC595_SH_PIN, MSBFIRST, value); // Send the byte
    digitalWrite(HC595_ST_PIN, HIGH); // Turn on the output — the data appears on the pins Q0–Q7
  }
  // 
  /*
  void sendBytesToShiftRegisters(const byte* data, size_t count) {
    digitalWrite(latchPin, LOW);
    for (int i = count - 1; i >= 0; i--) {
        shiftOut(dataPin, clockPin, MSBFIRST, data[i]);
    }
    digitalWrite(latchPin, HIGH);
  }
  byte values[9] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
  sendBytesToShiftRegisters(values, 2);
  */
}

namespace thermocouple_MAX6675
{
  void setting_pin_values_for_SPI()
  {
    pinMode(SPI_CLK_PIN, OUTPUT);
    digitalWrite(SPI_CLK_PIN, HIGH);
  }

  double readCelsius() //220-230 ms
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
