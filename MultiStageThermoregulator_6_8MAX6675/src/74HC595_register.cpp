#include "74HC595_register.h"

ShiftRegister74HC595::ShiftRegister74HC595(uint8_t DS, uint8_t ST, uint8_t SH) 
  : _DS_pin(DS), _ST_pin(ST), _SH_pin(SH) {}

void ShiftRegister74HC595::init() {
    pinMode(_DS_pin, OUTPUT);
    pinMode(_ST_pin, OUTPUT);
    pinMode(_SH_pin, OUTPUT);
    digitalWrite(_DS_pin, HIGH);
    digitalWrite(_ST_pin, HIGH);
    digitalWrite(_SH_pin, LOW);
}

void ShiftRegister74HC595::write(uint8_t data) {  // 40 mсs // The value is allowed to be from 0 to 255
    digitalWrite(_ST_pin, LOW);
    for (int i = 7; i >= 0; i--) {
      digitalWrite(_SH_pin, LOW);
      digitalWrite(_DS_pin, (data >> i) & 1);
      digitalWrite(_SH_pin, HIGH);
    }
    digitalWrite(_ST_pin, HIGH);
}

void ShiftRegister74HC595::clear() {
    write(0x00);
}

void ShiftRegister74HC595::set_0_on_DS_pin() {  // 20 mсs
    digitalWrite(_DS_pin, LOW);
    delayMicroseconds(20);
}
void ShiftRegister74HC595::set_1_on_DS_pin() {  // 20 mсs
    digitalWrite(_DS_pin, HIGH);
    delayMicroseconds(20);
}
void ShiftRegister74HC595::shift_bits() {  // 20 mсs
    digitalWrite(_SH_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_SH_pin, LOW);
    delayMicroseconds(10);
}
void ShiftRegister74HC595::save_data() {  // 20 mсs
    digitalWrite(_ST_pin, LOW);
    delayMicroseconds(10);
    digitalWrite(_ST_pin, HIGH);
    delayMicroseconds(10);
}
void ShiftRegister74HC595::set_11111111() {
    write(0xFF);
}
void ShiftRegister74HC595::add_0_to_reg() {    // 80 mсs
    set_0_on_DS_pin();
    shift_bits();
    set_1_on_DS_pin();
    save_data();
}
void ShiftRegister74HC595::shift_and_save_data() {    // 40 mсs
    shift_bits();
    save_data();
}

void ShiftRegister74HC595::debug_print_state() {
    Serial.print("DS: ");
    Serial.print(digitalRead(_DS_pin));
    Serial.print(" ST: ");
    Serial.print(digitalRead(_ST_pin));
    Serial.print(" SH: ");
    Serial.println(digitalRead(_SH_pin));
}