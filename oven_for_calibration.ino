#include "src/thermocouples.h"
#include "src/current_sensors.h"

current_sensors::descriptor_t current_sensor_descriptors[CURRENT_SENSORS_NUM] = {
    { 0x40, 0.1 },
    { 0x41, 0.1 },
    { 0x44, 0.1 }
};

void setup() {
  Serial.begin(9600);
  thermocouples::init();
  Serial.println("Thermocouples MAX6675 were initialized...");
  current_sensors::init(current_sensor_descriptors);
  Serial.println("Measuring current with INA219 ...");
}

void loop() {
  static String receivedData="";
  if (Serial.available() > 0) {
    receivedData = Serial.readString();
    receivedData.trim();
    //Serial.println("Received data: " + receivedData);
    Serial.flush();
  }
  
  if (receivedData=="read_one_point;") {
    thermocouples::read_all_temp();
    for (size_t i = 0; i < THERMOCOUPLES_NUM; i++)
    {
      Serial.print("Temperature #");
      Serial.print(i);
      Serial.print(":       ");
      Serial.print(thermocouples::get_temp_celsius(i));
      Serial.println(" \370");
    } 
    
    current_sensors::read_all_current(); 
        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
        {
            Serial.print("Current #");
            Serial.print(i);
            Serial.print(":       ");
            Serial.print(current_sensors::get_current_amps(i));
            Serial.println(" mA");
        } 
    delay(1000);
    receivedData = "";
  }
  
  if (receivedData=="read;") {
    thermocouples::read_all_temp();
    for (size_t i = 0; i < THERMOCOUPLES_NUM; i++)
    {
      Serial.print("Temperature #");
      Serial.print(i);
      Serial.print(":       ");
      Serial.print(thermocouples::get_temp_celsius(i));
      Serial.println(" \370");
    } 
    
    current_sensors::read_all_current(); 
        for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
        {
            Serial.print("Current #");
            Serial.print(i);
            Serial.print(":       ");
            Serial.print(current_sensors::get_current_amps(i));
            Serial.println(" mA");
        } 
    delay(1000);
  }
  
  if (receivedData=="stop_reading;") {
    thermocouples::stop_reading_temp();
    //Serial.println("Reading stoped");
    receivedData="";
  }
}

