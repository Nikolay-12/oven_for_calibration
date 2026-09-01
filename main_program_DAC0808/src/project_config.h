#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <SPI.h>

// ============================================
//  АППАРАТНЫЕ КОНСТАНТЫ
// ============================================
// Основные настройки для ЦАП
static constexpr int DAC_ADDR = 0x60;       // I2C адрес для MCP4725
static constexpr int MAX_VALUE_12BIT = 4095;  // Максимальное значение сигнала для ЦАП (12-битного) - MCP4725
static constexpr uint8_t MAX_VALUE_8BIT = 255;  // Максимальное значение сигнала для ЦАП (8-битного) - DAC0808
static constexpr float Vref = 3.3f; // референсное напряжение ЦАП
static constexpr uint8_t DAC_OUTPUT_PIN = 4; 
static constexpr uint8_t I2C_SDA_PIN = 15;
static constexpr uint8_t I2C_SCL_PIN = 16;

// Настройка пинов для термопары и твердотельного реле
static constexpr uint8_t SPI_CLK_PIN = 12; // Пин SCK термопары для Arduino Nano 13, для ESP32S3 12 или 36
static constexpr uint8_t SPI_MISO_PIN = 13; // ПИН SO термопары для Arduino Nano 12, для ESP32S3 13 или 37
static constexpr uint8_t SPI_CS_PIN = 10; // Пин CS термопары для Arduino Nano 10, для ESP32S3 10 или 39
static constexpr uint8_t OVEN_PWM_PIN = 18; // Пин для твердотельного реле, на него подключаем нагреватель
static constexpr uint8_t MOSFET_PIN = 17; // Пин для MOSFET-транзистора, переключающий режим работы системы (I(T) или I(V))

// Архитектурные особенности подлюченных термопар
static constexpr uint8_t REGISTERS_NUM = 4; 
static constexpr uint8_t TOTAL_OUTPUT_NUM = REGISTERS_NUM * 8; 
static constexpr uint8_t THERMOCOUPLES_NUM = 25; 
static constexpr uint8_t FIRST_USEFUL_PIN = 1; // В проекте: восьмой пин первого регистра используется первым

// Настройка пинов для управления регистрами термопар
static constexpr uint8_t HC595_DS_PIN = 7; // Пин данных для сдвиговых регистов ("путешествующий ноль")
static constexpr uint8_t HC595_ST_PIN = 6; // Пин для защелкивания выходов сдвиговых регистов
static constexpr uint8_t HC595_SH_PIN = 5; // Пин тактовых импульсов для сдвиговых регистов
//static constexpr uint8_t ESP_MUX8to2_1 = 46; // Пин №1 переключения между каналами для сдвоенного мультиплексора 4 в 1
//static constexpr uint8_t ESP_MUX8to2_2 = 9; // Пин №2 переключения между каналами для сдвоенного мультиплексора 4 в 1
static constexpr uint8_t DAC_HC595_DS_PIN = 7; // Пин тактовых импульсов для сдвигового региста ЦАП
static constexpr uint8_t DAC_HC595_ST_PIN = 6; // Пин для защелкивания выходов сдвигового региста ЦАП
static constexpr uint8_t DAC_HC595_SH_PIN = 5; // Пин тактовых импульсов для сдвигового региста ЦАП
//static constexpr uint8_t HC595_OE_PIN = 8; // Пин подключения и отключения выхода со сдвигового региста (0 - вкл, 1 - выкл)

static constexpr float MAX6675_READING_TIMEOUT = 0.25f; // Таймаут на чтение температуры с MAX6675 (>220 мс)


// Настройка параметров для ПИД-регулятора
const float DEFAULT_KP = 10.0f; // Дефолтное значение для коэффициента Kp
const float DEFAULT_KI = 0.3f; // Дефолтное значение для коэффициента Ki
const float DEFAULT_KD = 0.8f; // Дефолтное значение для коэффициента Kd
const float DEFAULT_DT = 0.1f;  // Шаг дискретизации 100 мс
const float DEFAULT_stabilization_time = 5.0f; // Время, которое необходимо для подтверждения стабилизации температуры (с)
const float DEFAULT_holding_time = 10.0f; // Время удержания данной температуры (в это время идет сбор всех данных с датчиков) (с)
const float DEFAULT_temp_tolerance = 1.0f; // Температурная погрешность
const float DEFAULT_max_stab_time = 30.0f; // максимальное время на одной стадии
const float DEFAULT_TEMPERATURE_STEP = 10.0f;
const float DEFAULT_TEMPERATURE_MIN = 30.0f;
const float DEFAULT_TEMPERATURE_MAX = 100.0f;
const int VOLTAGE_MIN_FOR_IT_MODE = 10;

// Настройка параметров для многостадийного ЦАП
const float DEFAULT_VOLTAGE_STEP = 0.1f;
const float DEFAULT_VOLTAGE_MIN = 0.0f;
const float DEFAULT_VOLTAGE_MAX = 1.0f;

// Глобальные буферы температур и токов
extern float tempReadings[20];
//float voltReadings[20];
extern float currentReadings[20];

const uint8_t CURRENT_SENSORS_NUM = 16;

// Структура настроек датчика измерения тока INA219 (адрес на шине I2C, а также шунтовые сопротивления)
struct INA219_descriptor {
    uint8_t address;  // Адрес на шине I2C
    float shunt_R1_ohms;  // Шунтовое сопротивление №1 (Ом)
    float shunt_R2_ohms;  // Шунтовое сопротивление №2 (Ом)
    uint8_t MUX_channel;
};

// Численные значения настроек для всех 16 сенсоров
static constexpr INA219_descriptor CURRENT_SENSOR_DESCRIPTORS[CURRENT_SENSORS_NUM] = {
    {0x40,  0.1f,  20.0f, 0},      {0x41,  0.1f,  20.0f, 0},      {0x44,  0.1f,  20.0f, 0},      {0x45,  0.1f,  20.0f, 0},       // Канал 0: сенсоры 1-4
    {0x40,  0.1f,  20.0f, 1},      {0x41,  0.1f,  20.0f, 1},      {0x44,  0.1f,  20.0f, 1},      {0x45,  0.1f,  20.0f, 1},       // Канал 1: сенсоры 5-8
    {0x40,  0.1f,  20.0f, 2},      {0x41,  0.1f,  20.0f, 2},      {0x44,  0.1f,  20.0f, 2},      {0x45,  0.1f,  20.0f, 2},       // Канал 2: сенсоры 9-12
    {0x40,  0.1f,  20.0f, 3},      {0x41,  0.1f,  20.0f, 3},      {0x44,  0.1f,  20.0f, 3},      {0x45,  0.1f,  20.0f, 3}       // Канал 3: сенсоры 13-16
};
// Примечание: 0x40 (перемычки не установлены); 0x41 (перемычка A0); 0x44 (перемычка A1); 0x45 (перемычки A0 и A1).


static constexpr uint8_t ESP_MUX8to2_S0 = 46;  // Младший бит
static constexpr uint8_t ESP_MUX8to2_S1 = 9;  // Старший бит

static constexpr uint8_t MUX_CHANNELS = 4; // Количество каналов в мультиплексоре


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