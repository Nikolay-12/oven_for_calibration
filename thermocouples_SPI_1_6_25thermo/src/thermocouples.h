#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "74HC595_register.h"
#include "MAX6675_thermocouple.h"

//static constexpr uint8_t REGISTERS_NUM = 4; 
static constexpr uint8_t TOTAL_OUTPUT_NUM = REGISTERS_NUM * 8; 
static constexpr uint8_t THERMOCOUPLES_NUM = 25; 
//static constexpr uint8_t THERMOCOUPLES_NUM = 25; 
static constexpr uint8_t FIRST_USEFUL_PIN = 1; // В проекте: восьмой пин первого регистра используется первым
static constexpr uint8_t CLK_PIN = 12; // Пин SCK термопар в ESP32
static constexpr uint8_t MISO_PIN = 13; // ПИН MISO термопар в ESP32

// Структура для калибровочных коэффициентов
struct CalibrationCoeffs {
    float k;  // Множитель
    float b;  // Смещение
};

// Калибровочные коэффициенты для каждой термопары
// y = k * x + b, где x - измеренная температура, y - скорректированная
static constexpr CalibrationCoeffs CALIBRATION[THERMOCOUPLES_NUM] = {
    {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},   // 1-5
    {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},   // 6-10
    {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},   // 11-15
    {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},   // 16-20
    {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0},      {1.000,  0.0}   // 21-25
};

namespace thermocouples
{
    void init(ShiftRegister74HC595* reg);
    void read_all_temp_once(bool calibrate);
    void read_all_temp(bool calibrate);
    void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index);
    void read_temp_from_one_thermocouple(uint8_t thermocouple_index);
    void stop_reading_temp();
    double get_temp_celsius(uint8_t thermocouple_index);
    double get_average_temp_celsius();
    void average_temp_celsius(bool calibrate);
    void send_temp_point(uint8_t thermocouple_index, bool calibrate);
    void send_all_temp(bool calibrate);
    void send_temp_array(bool calibrate);

    float get_calibrated_temp(uint8_t thermocouple_index);
}

