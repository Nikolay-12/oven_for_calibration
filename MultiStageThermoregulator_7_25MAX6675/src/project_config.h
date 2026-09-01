#pragma once

#include <Arduino.h>
#include <stdint.h>

// ============================================
//  АППАРАТНЫЕ КОНСТАНТЫ
// ============================================
// Основные настройки для ЦАП
static constexpr uint8_t DAC_ADDR = 0x60;       // I2C адрес для MCP4725
static constexpr int MAX_VALUE = 4095;  // Максимальное значение сигнала для ЦАП (12-битного)
static constexpr float Vref = 3.3f; // референсное напряжение ЦАП
static constexpr uint8_t output_DAC_pin = 4; 
static constexpr uint8_t I2C_SDA_PIN = 15;
static constexpr uint8_t I2C_SCL_PIN = 16;

// Настройка пинов для термопары и твердотельного реле
static constexpr uint8_t CLK_PIN = 12; // Пин SCK термопары для Arduino Nano 13, для ESP32S3 12 или 36
static constexpr uint8_t DATA_PIN = 13; // ПИН SO термопары для Arduino Nano 12, для ESP32S3 13 или 37
static constexpr uint8_t CS_PIN = 10; // Пин CS термопары для Arduino Nano 10, для ESP32S3 10 или 39
static constexpr uint8_t RELAY_PIN = 3; // Пин для твердотельного реле, на него подключаем нагреватель

static constexpr uint8_t HC595_DS_PIN = 7; // Пин данных для сдвиговых регистов ("путешествующий ноль")
static constexpr uint8_t HC595_ST_PIN = 6; // Пин для защелкивания выходов сдвиговых регистов
static constexpr uint8_t HC595_SH_PIN = 5; // Пин тактовых импульсов для сдвиговых регистов

// Архитектурные особенности подлюченных термопар
static constexpr uint8_t REGISTERS_NUM = 4; 
static constexpr uint8_t TOTAL_OUTPUT_NUM = REGISTERS_NUM * 8; 
static constexpr uint8_t THERMOCOUPLES_NUM = 25; 
static constexpr uint8_t FIRST_USEFUL_PIN = 1; // В проекте: восьмой пин первого регистра используется первым

// Важные константы для работы с устройства
static constexpr float MAX6675_READING_TIMEOUT = 0.25f; //Необходимый таймаут чтения с термопары (время обработки сигнала)



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