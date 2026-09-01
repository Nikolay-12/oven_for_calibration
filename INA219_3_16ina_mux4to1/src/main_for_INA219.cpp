#include "main_for_INA219.h"
#include "MUX4to1.h"
#include "auxiliary_file.h"

namespace INA219_dop
{
  uint32_t tmr;
  bool ina_initialization_ok = false;
  bool state = false;
  enum Mode  {IDLE = 0, 
              READ_N = 1, // Режим измерения показаний N раз
              READING = 2}; // Режим измеренияпоказаний до прерывания
  Mode mode = Mode::IDLE;
  float current_mA;
  int cycles_num = 10;
  float period_s = 2;
  int count = 0;
  

  void init() {
    //
  }

  void updateMeasurementSettings(const int cycles, const float periodization) {
    cycles_num = cycles;
    period_s = periodization;
    Serial.print("Measurement parameters: measurement cycles = "); Serial.print(cycles_num); 
    Serial.print(" ; measurement timeout = "); Serial.print(period_s);  Serial.println(" s");
  }

  void read_all_currents() {
    unsigned long startTime = millis();
    current_sensors::read_all_16();
    unsigned long readTime = millis() - startTime;
    
    // Вывод значений
    Serial.print("Токи (мА): ");
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
        float current_mA = current_sensors::get_current_amps(i) * 1000.0f;
        Serial.print(current_mA, 2); // 2 знака после запятой
        Serial.print(" ");
        
        // Для удобства чтения - перенос после каждого 4-го значения
        if ((i + 1) % 4 == 0 && i < CURRENT_SENSORS_NUM - 1) {
            Serial.print("| ");
        }
    }
    Serial.print(" (время: ");
    Serial.print(readTime);
    Serial.println(" мс)");
  }
  void readCurrents() {  // Чтение токов с датчиков INA219
    current_sensors::read_all();
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
    {
      Serial.print("Current #");
      Serial.print(i);
      Serial.print(":       ");
      Serial.print(current_sensors::get_current_amps(i)*1000.0f, 2);
      Serial.println(" mA");
    } 
  }
  void readCurrentArray() {  // Чтение токов с датчиков INA219
    current_sensors::read_all();
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
    {
      Serial.print(current_sensors::get_current_amps(i)*1000.0f, 2);
      Serial.print(" ");
    } 
    Serial.println("");
  }

  // Функция для чтения конкретного датчика по команде
  void readSingleSensor(size_t index) {
      if (index < CURRENT_SENSORS_NUM) {
          current_sensors::read_single(index);
          float current = current_sensors::get_current_amps(index)*1000.0f;
          Serial.print("Датчик ");
          Serial.print(index);
          Serial.print(" (адрес 0x");
          Serial.print(CURRENT_SENSOR_DESCRIPTORS[index].address, HEX);
          Serial.print(", канал ");
          Serial.print(CURRENT_SENSOR_DESCRIPTORS[index].MUX_channel);
          Serial.print("): ");
          Serial.print(current, 2);
          Serial.println(" мА");
      } else {
          Serial.println("Неверный индекс датчика");
      }
  }

  // Функция для чтения всех датчиков на конкретном канале
  void readChannel(uint8_t channel) {
      if (channel >= 4) {
          Serial.println("Неверный канал (0-3)");
          return;
      }
      
      Serial.print("Чтение канала ");
      Serial.println(channel);
      
      // Переключаем мультиплексор на нужный канал
      MuxController::selectChannel(channel);
      delay(10);
      
      // Читаем все датчики на этом канале
      for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
          if (CURRENT_SENSOR_DESCRIPTORS[i].MUX_channel == channel) {
              current_sensors::read_single(i);
              float current = current_sensors::get_current_amps(i)*1000.0f;
              Serial.print("  Датчик ");
              Serial.print(i);
              Serial.print(" (0x");
              Serial.print(CURRENT_SENSOR_DESCRIPTORS[i].address, HEX);
              Serial.print("): ");
              Serial.print(current, 2);
              Serial.println(" мА");
          }
      }
  }

  void startEndlessReading() {
    state = true;
    mode = Mode::READING;
    Serial.println("Начинается измерение величин токов (в мА) до прерывания...");
  }

  void currentReading() {
    unsigned long now = millis();

    if (now - tmr >= (period_s * 1000)) {
      tmr = millis();
      //readCurrents();
      readCurrentArray();
      //read_all_currents();
    }
    delay(1);
  }

  void startReadingSeveralTimes() {
    count = 0;
    state = true;
    mode = Mode::READ_N;
    Serial.print("Начинается измерение величин токов (в мА), состоящее из "); Serial.print(cycles_num); Serial.println(" циклов...");
  }

  void readCurrentSeveralTimes() {
    unsigned long now = millis();

    if (now - tmr >= (period_s * 1000)) {
      tmr = millis();
      Serial.print("Цикл № "); Serial.println(count + 1);
      //readCurrents();
      readCurrentArray();
      //read_all_currents();
      count++;
    }
    delay(1);
    
    if (count >= cycles_num) {
        off();
        Serial.println("Измерение завершено!");
    }
  }

  void on() {
    state = true;
  }
  void off() {
    state = false;
    mode = Mode::IDLE;
  }

  bool getState() {
    return state;
  }
  uint8_t getMode() {
    return static_cast<uint8_t>(mode);
  }
};