#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "project_config.h"


class ShiftRegister74HC595
{
private:
    uint8_t _DS_pin;
    uint8_t _ST_pin;
    uint8_t _SH_pin;
public:
    ShiftRegister74HC595(uint8_t DS, uint8_t ST, uint8_t SH);
    void init();
    void write(uint8_t value, bool latchEnabled); // 40 mсs   // The value is allowed to be from 0 to 255
    void writeRegisters(uint32_t data);
    void clear();
    void set_0_on_DS_pin();
    void set_1_on_DS_pin();
    void shift_bits();
    void save_data();
    void set_11111111();
    void add_0_to_reg();
    void shift_and_save_data();
    void debug_print_state();
};