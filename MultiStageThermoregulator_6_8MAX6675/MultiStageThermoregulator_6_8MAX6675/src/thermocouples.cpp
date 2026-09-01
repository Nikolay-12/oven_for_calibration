#include "thermocouples.h"
#include <SPI.h>

namespace MAX6675 = thermocouple_MAX6675;
static ShiftRegister74HC595* _reg = nullptr;

namespace thermocouples
{
  double average_temp;
  struct instance_t
  {
    double temp_celsius;
  };
  instance_t instances[THERMOCOUPLES_NUM];

  void init(ShiftRegister74HC595* reg) 
  {
    _reg = reg;
    _reg->init();
    MAX6675::init();
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    //SPI.setDataMode(SPI_MODE0);
    //SPI.setBitOrder(MSBFIRST);
    _reg->write(0xFF);
  }

  void read_all_temp_once() //20 mсs + 230 ms + 60*7 mсs + 230*7 ms = 1840 ms   // for 25 - 5750 ms
  { 
    // Начинаем с термопары 1 (бит 0 = 0)
    _reg->write(0xFE);  // 0b11111110 - только термопара 1 включена
    delay(10);
    instances[0].temp_celsius = MAX6675::readCelsius();
    
    // Читаем остальные термопары
    for (size_t i = 1; i < THERMOCOUPLES_NUM; i++) {
      uint8_t byte_to_send = 0xFF ^ (1 << i);  // Бит i = 0, остальные = 1
      _reg->write(byte_to_send);
      delay(10);
      instances[i].temp_celsius = MAX6675::readCelsius();
    }
    
    _reg->write(0xFF);  // Отключаем все
    average_temp_celsius();
  }
  void read_all_temp() {
    read_all_temp_once();
  }

  void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index) // 230 ms
  {
    // Создаем байт: 0 в позиции thermocouple_index-1, остальные 1
    uint8_t byte_to_send = 0xFF ^ (1 << (thermocouple_index - 1));
    
    _reg->write(byte_to_send);
    delay(10);  // Даем время на переключение
    
    instances[thermocouple_index-1].temp_celsius = MAX6675::readCelsius();
    _reg->write(0xFF);  // Отключаем все
  }
  void read_temp_from_one_thermocouple(uint8_t thermocouple_index) 
  {
    read_temp_from_one_thermocouple_once(thermocouple_index);
  }

  void stop_reading_temp()
  {
    _reg->write(0xFF);
    Serial.println("Reading stoped.");
  }

  double get_temp_celsius(uint8_t thermocouple_index)
  {  
    if (thermocouple_index > THERMOCOUPLES_NUM) return NAN;
    return instances[thermocouple_index-1].temp_celsius;
  }
  double get_average_temp_celsius()
  {
    return average_temp;
  }

  void average_temp_celsius()
  {
    double valid_sum = 0;
    int valid_count = 0;
    for (size_t i = 0; i < THERMOCOUPLES_NUM; i++) {
      if ((instances[i].temp_celsius > 10)&&(instances[i].temp_celsius < 700)){
        valid_sum += instances[i].temp_celsius;
        valid_count++;
      }
    }
    if (valid_count == 0) {
      average_temp = NAN;
    }
    else {
      average_temp = valid_sum / valid_count;
    }
  }

  void send_temp_point(uint8_t thermocouple_index){
    Serial.print("Temperature #");
    Serial.print(thermocouple_index);
    Serial.print(":       ");
    Serial.print(get_temp_celsius(thermocouple_index));
    Serial.println(" °C");
    //delay(1000);
  }
  void send_all_temp(){
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++)
    {
      Serial.print("Temperature #");
      Serial.print(i);
      Serial.print(":       ");
      Serial.print(get_temp_celsius(i));
      Serial.println(" °C");
    }
    Serial.print("Average temperature: ");
    Serial.println(get_average_temp_celsius());
    //delay(1000);
  }
  void send_temp_array(){
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++)
    {
      Serial.print(get_temp_celsius(i));
      Serial.print(" ");
    }
    Serial.print("  ");
    Serial.println(get_average_temp_celsius());
    //delay(1000);
  }
}
