#include "src/thermocouples.h"
#include "src/74HC595_register.h"

#define HC595_DS_PIN 7 // Пин данных для сдвиговых регистов ("путешествующий ноль")
#define HC595_STcp_PIN 6 // Пин для защелкивания выходов сдвиговых регистов
#define HC595_SHcp_PIN 5 // Пин тактовых импульсов для сдвиговых регистов
ShiftRegister74HC595 reg(HC595_DS_PIN, HC595_STcp_PIN, HC595_SHcp_PIN);

void setup(void) {
  Serial.begin(9600);
  Serial.println("System started");
  Serial.println("Using 4x 74HC595 registers");
  Serial.println("First 7 pins (Q0-Q6) are unused and skipped");
  Serial.println("Thermocouples 1-25 are on pins Q7, Q8-Q31");
  thermocouples::init(&reg);
  Serial.println("Thermocouples MAX6675 were initialized...");
}

void loop(void) {
  static String receivedData="";
  if (Serial.available() > 0) {
    receivedData = Serial.readString();
    receivedData.trim();
    Serial.flush();
  }

  if (receivedData=="readOnePointOnce;") { // read one temp from thermocouple №4
    thermocouples::read_temp_from_one_thermocouple_once(4);
    thermocouples::send_temp_point(4, true);
    receivedData = "";
  }
  if (receivedData=="readOnePoint;") { // read temp from thermocouple №4
    thermocouples::read_temp_from_one_thermocouple(4);
    thermocouples::send_temp_point(4, true);
    delay(250);
  }

  if (receivedData=="readAllTempOnce;") { // read all temp once
    thermocouples::read_all_temp_once(true);
    thermocouples::send_temp_array(true);
    receivedData = "";
  }
  
  if (receivedData=="readAllTemp;") { // read all temp before interruption
    thermocouples::read_all_temp(true);
    thermocouples::send_temp_array(true);
    delay(250);
  }
  
  if (receivedData=="stop_reading;") {
    thermocouples::stop_reading_temp();
    receivedData="";
  }
}
/*
Commands:
writeByteToReg;
readOnePointOnce;
readOnePoint;
readAllTempOnce;
readAllTemp;
stop_reading;
*/