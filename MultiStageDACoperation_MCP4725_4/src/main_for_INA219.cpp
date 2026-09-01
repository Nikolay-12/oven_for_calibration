#include "main_for_INA219.h"
#include "main_for_DAC_MCP4725.h"
#include "auxiliary_file.h"

namespace INA219_dop
{
  uint32_t INAtmr;
  bool ina_initialization_ok = false;
  bool INAstate = false;
  bool INAmode = false;
  uint8_t address = 0x40;
  float shunt_resistance_ohms = 0.1;
  float current_mA;
  int cycles_num = 10;
  float period_s = 2;
  int count = 0;
  

  void init() {
    initI2C(I2C_SDA_PIN, I2C_SCL_PIN);
    ina_initialization_ok = false;
    current_sensors::init_by_address(address, shunt_resistance_ohms, ina_initialization_ok);
    delay(1000);
  }

  void updateMeasurementSettings(const int cycles, const float periodization) {
    cycles_num = cycles;
    period_s = periodization;
    DAC_MCP4725::updateMeasurementSettings(cycles_num, period_s);
    Serial.print("Measurement parameters: measurement cycles = "); Serial.print(cycles_num); 
    Serial.print(" ; measurement timeout = "); Serial.print(period_s);  Serial.println(" s");
  }

  float readINAcurrent() {  // Чтение тока с датчика INA219
    current_mA = current_sensors::get_current_amps_by_address(address, shunt_resistance_ohms, ina_initialization_ok) * 1000;
    //Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    return current_mA;
  }

  void startEndlessReading() { // const float periodization
    INAstate = true;
    INAmode = 0;
  }

  void currentReading() {
    unsigned long now = millis();

    if (now - INAtmr >= (period_s * 1000)) {
      INAtmr = millis();
      readINAcurrent();
    }
    delay(1);
  }

  void startReadingSeveralTimes() { // const int cycles, const float periodization
    count = 0;
    INAstate = true;
    INAmode = 1;
  }

  void readCurrentSeveralTimes() {
    unsigned long now = millis();

    if (now - INAtmr >= (period_s * 1000)) {
      INAtmr = millis();
      readINAcurrent();
      count++;
    }
    delay(1);
    
    if (count >= cycles_num) {
        INAstate = false;
        Serial.println("Reading done!");
    }
  }

  void INA219on() {
    INAstate = true;
  }
  void INA219off() {
    INAstate = false;
  }

  bool getState() {
    return INAstate;
  }
  bool getMode() {
    return INAmode;
  }
};