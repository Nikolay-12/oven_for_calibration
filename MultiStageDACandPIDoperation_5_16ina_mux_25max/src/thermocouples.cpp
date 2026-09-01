#include <SPI.h>
#include "thermocouples.h"
#include "System_State.h"


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

  void init(ShiftRegister74HC595* reg)  // Команда для инициализации термопар (+ сдвигового регистр)
  {
    _reg = reg;
    _reg->init();
    MAX6675::init();
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    //SPI.setDataMode(SPI_MODE0);
    //SPI.setBitOrder(MSBFIRST);
    _reg->write(0xFFFFFFFF, true);
  }

  void read_all_temp_once(bool calibrate) //20 mсs + 230 ms + 60*7 mсs + 230*7 ms = 1840 ms   // for 25 - 5750 ms
  { 
    // Сразу стартуем с позиции 7
    uint32_t zeroPosition = 1 << FIRST_USEFUL_PIN - 1; // начинаем с 0b...10000000
    
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++) {
        uint32_t message = ~zeroPosition;
        _reg->writeRegisters(message);
        delayMicroseconds(50);

        instances[i-1].temp_celsius = MAX6675::readCelsius();
        
        zeroPosition <<= 1; // Сдвигаем ноль
    }
    
    _reg->writeRegisters(0xFFFFFFFF);
    delayMicroseconds(500);
    average_temp_celsius(calibrate); // усреднение полученного массива температур (с возможностью калибровки)
  }

  void read_all_temp(bool calibrate) {
    read_all_temp_once(calibrate);
  }

  void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index) // 230 ms
  {
    // Сразу стартуем с позиции thermocouple_index - 1 + FIRST_USEFUL_PIN - 1
    uint32_t zeroPosition = 1 << thermocouple_index - 1 + FIRST_USEFUL_PIN - 1; // начинаем с 0b...10000000
    uint32_t message = ~zeroPosition;
    _reg->writeRegisters(message);
    delayMicroseconds(50); 

    instances[thermocouple_index-1].temp_celsius = MAX6675::readCelsius();
    _reg->writeRegisters(0xFFFFFFFF);
    delayMicroseconds(500);
  }
  void read_temp_from_one_thermocouple(uint8_t thermocouple_index) 
  {
    read_temp_from_one_thermocouple_once(thermocouple_index);
  }
  
  void stop_reading_temp() // Команда для прерывания чтения с термопар
  {
    _reg->writeRegisters(0xFFFFFFFF);
    Serial.println("Reading stoped.");
    SystemState::free();
  }

  // Команды для получения температуры с выбранной термопары и для получения усредненной температуры
  double get_temp_celsius(uint8_t thermocouple_index)  // Команды для получения температуры с выбранной термопары
  {  
    if (thermocouple_index > THERMOCOUPLES_NUM) return NAN;
    return instances[thermocouple_index-1].temp_celsius;
  }
  double get_average_temp_celsius()  // Команды для получения усредненной температуры
  {
    return average_temp;
  }

  // Команда для усреднения температуры по всем термопарам
  void average_temp_celsius(bool calibrate) {
    double valid_sum = 0;
    int valid_count = 0;
    for (size_t i = 0; i < THERMOCOUPLES_NUM; i++) {
      float temp = 0.0f;
      if (calibrate) {
        temp = get_calibrated_temp(i + 1);
      }
      else {
        temp = get_temp_celsius(i + 1);
      }
      if (temp > 10 && temp < 700) {
        valid_sum += temp;
        valid_count++;
      }
    }
    if (valid_count == 0) {
      average_temp = NAN;
    } else {
      average_temp = valid_sum / valid_count;
    }
    //Serial.print("valid_count: "); Serial.println(valid_count);
  }

  // Команды для вывода в COM-порт температур с термопар
  void send_temp_point(uint8_t thermocouple_index, bool calibrate) {  // Команда для вывода в COM-порт температуру с выбранной термопары (по индексу)
    Serial.print("Temperature #");
    Serial.print(thermocouple_index);
    Serial.print(":       ");
    if (calibrate) {
      Serial.print(get_calibrated_temp(thermocouple_index), 2);
    }
    else {
      Serial.print(get_temp_celsius(thermocouple_index), 2);
    }
    Serial.println(" °C");
  }
  void send_all_temp(bool calibrate) {  // Команда для вывода в COM-порт всех температур (в формате "Temperature #N: 34.50 °C") + средняя температура
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++)
    {
      Serial.print("Temperature #");
      Serial.print(i);
      Serial.print(":       ");
      if (calibrate) {
        Serial.print(get_calibrated_temp(i), 2);
      }
      else {
        Serial.print(get_temp_celsius(i), 2);
      }
      Serial.println(" °C");
    }
    Serial.print("Average temperature: ");
    Serial.println(get_average_temp_celsius());
  }
  void send_temp_array(bool calibrate) {  // Команда для вывода в COM-порт всех температур (в формате "25.00 34.50 ...")
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++)
    {
      if (calibrate) {
        Serial.print(get_calibrated_temp(i), 2);
      }
      else {
        Serial.print(get_temp_celsius(i), 2);
      }
      Serial.print(" ");
    }
  }
  void send_temp_array_and_avr(bool calibrate) {  // Команда для вывода в COM-порт всех температур (в формате "25.00 34.50 ...") + средняя температура
    for (size_t i = 1; i <= THERMOCOUPLES_NUM; i++)
    {
      if (calibrate) {
        Serial.print(get_calibrated_temp(i), 2);
      }
      else {
        Serial.print(get_temp_celsius(i), 2);
      }
      Serial.print(" ");
    }
    Serial.print("  ");
    Serial.println(get_average_temp_celsius());
  }

    // Функция для получения откалиброванной температуры
    float get_calibrated_temp(uint8_t thermocouple_index) {
        if (thermocouple_index == 0 || thermocouple_index > THERMOCOUPLES_NUM) return NAN;
        
        float raw_temp = get_temp_celsius(thermocouple_index);
        if (isnan(raw_temp)) return NAN;
        
        return CALIBRATION[thermocouple_index - 1].k * raw_temp + CALIBRATION[thermocouple_index - 1].b; // Применяем калибровку из массива настроек
    }
}
