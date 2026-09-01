#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>

// ============================================
//  АППАРАТНЫЕ КОНСТАНТЫ
// ============================================
// Основные настройки для ЦАП
static constexpr int DAC_ADDR = 0x60;       // I2C адрес для MCP4725
static constexpr int MAX_VALUE = 4095;  // Максимальное значение сигнала для ЦАП (12-битного)
static constexpr float Vref = 3.3f; // референсное напряжение ЦАП
static constexpr uint8_t output_DAC_pin = 4; 
static constexpr uint8_t I2C_SDA_PIN = 15;
static constexpr uint8_t I2C_SCL_PIN = 16;