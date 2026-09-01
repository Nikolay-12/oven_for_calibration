#include "main_for_INA219.h"
#include "main_for_DAC_MCP4725.h"
#include "Thermodat.h"
#include "auxiliary_file.h"
#include "System_State.h"

namespace INA219controller
{
  uint32_t tmr;
  bool ina_initialization_ok = false;
  bool state = false;
  enum Mode  {IDLE = 0, 
              READ_N = 1, // Режим измерения показаний N раз
              READING = 2}; // Режим измеренияпоказаний до прерывания
  Mode mode = Mode::IDLE;
  float current_mA;
  int cycles_num = DEFAULT_MEASUREMENT_CYCLES;
  float period_s = DEFAULT_MEASUREMENT_PERIOD;
  int count = 0;
  
  //  Команда для инициализации шины I²С и сенсоров
  void init() {
    initI2C(I2C_SDA_PIN, I2C_SCL_PIN);
    ina_initialization_ok = false;
    Serial.println("Measuring current with 16 INA219 sensors...");
    current_sensors::init(CURRENT_SENSOR_DESCRIPTORS);
    Serial.println("Initializing was completed. The system is ready to start.");
    //current_sensors::init_by_address(address, shunt_resistance_ohms, ina_initialization_ok);
    delay(1000);
  }

  //  Команда для установления настроек считывания с сенсоров
  void updateMeasurementSettings(const int cycles, const float periodization) {
    cycles_num = cycles;
    period_s = periodization;
    DAC_MCP4725::updateMeasurementSettings(cycles_num, period_s);
    thermoRegulator::updateMeasurementSettings(cycles_num, period_s);
    Serial.print("Measurement parameters: measurement cycles = "); Serial.print(cycles_num); 
    Serial.print(" ; measurement timeout = "); Serial.print(period_s);  Serial.println(" s");
  }
  
  //  Команды для чтения данных / их вывода в Serial порт
  float readINAcurrent(uint8_t addr, float shunt_ohms, bool ina_initialization) {  // Чтение тока с датчика INA219 (адрес, шунт, инициализация)
    current_mA = current_sensors::get_current_amps_by_address(addr, shunt_ohms, ina_initialization) * 1000;
    //Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    return current_mA;
  }

  void read_all_currents() {  // Чтение токов со всех датчиков (расширенный вывод)
    unsigned long startTime = millis();
    current_sensors::read_all_16(); // Для чтения данных с 16 датчиков с переключениями мультиплексора
    unsigned long readTime = millis() - startTime;
    
    Serial.print("Токи (мА): ");
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
        float current_mA = current_sensors::get_current_amps(i) * 1000.0f;
        Serial.print(current_mA, 2);
        Serial.print(" ");
        
        // Для удобства чтения - перенос после каждого 4-го значения
        if ((i + 1) % 4 == 0 && i < CURRENT_SENSORS_NUM - 1) {
            Serial.print("| ");
        }
    }
    Serial.print(" (time: ");
    Serial.print(readTime);
    Serial.println(" ms)");
  }
  void readCurrents() {  // Чтение токов со всех датчиков (вывод в фотмате: "Current #N: 0.45 mA")
    //current_sensors::read_all_on_one_channel(); // Для чтения с датчиков на одном канале
    current_sensors::read_all_16(); 
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
    {
      Serial.print("Current #");
      Serial.print(i);
      Serial.print(":       ");
      Serial.print(current_sensors::get_current_amps(i)*1000.0f, 2);
      Serial.println(" mA");
    } 
  }
  void readCurrentArray() {  // Чтение токов со всех датчиков (вывод в фотмате: "0.24 1.67 ...")
    //current_sensors::read_all_on_one_channel();
    current_sensors::read_all_16(); 
    for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++)
    {
      Serial.print(current_sensors::get_current_amps(i)*1000.0f, 2);
      Serial.print(" ");
    } 
    Serial.println("");
  }

  void readSingleSensor(size_t index) { // Чтение тока с датчика INA219 по индексу (расширенный вывод)
      if (index < CURRENT_SENSORS_NUM) {
          current_sensors::read_single(index);
          float current = current_sensors::get_current_amps(index)*1000.0f;
          Serial.print("Sensor ");
          Serial.print(index);
          Serial.print(" (address 0x");
          Serial.print(CURRENT_SENSOR_DESCRIPTORS[index].address, HEX);
          Serial.print(", channel ");
          Serial.print(CURRENT_SENSOR_DESCRIPTORS[index].MUX_channel);
          Serial.print("): ");
          Serial.print(current, 2);
          Serial.println(" mA");
      } else {
          Serial.println("Invalid sensor index!");
      }
  }

  void readChannel(uint8_t channel) { // Чтение 4-х значений с датчиков INA219 на одном каналем (расширенный вывод)
      if (channel >= 4) {
          Serial.println("Wrong channel (0-3)");
          return;
      }
      
      Serial.print("Reading channel ");
      Serial.println(channel);
      MuxController::selectChannel(channel);
      delay(10);
      
      for (size_t i = 0; i < CURRENT_SENSORS_NUM; i++) {
          if (CURRENT_SENSOR_DESCRIPTORS[i].MUX_channel == channel) {
              current_sensors::read_single(i);
              float current = current_sensors::get_current_amps(i)*1000.0f;
              Serial.print("  Sensor ");
              Serial.print(i);
              Serial.print(" (0x");
              Serial.print(CURRENT_SENSOR_DESCRIPTORS[i].address, HEX);
              Serial.print("): ");
              Serial.print(current, 2);
              Serial.println(" mA");
          }
      }
  }

  //  Команды для запуска и остановки чтения данных / их вывода в Serial порт
  void startEndlessReading() {
    state = true;
    mode = Mode::READING;
    Serial.println("The measurement of current values ​​(in mA) begins...");
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
    Serial.print("The measurement of the current value (in mA) begins, consisting of "); Serial.print(cycles_num); Serial.println(" cycles...");
  }

  void readCurrentSeveralTimes() {
    unsigned long now = millis();

    if (now - tmr >= (period_s * 1000)) {
      tmr = millis();
      Serial.print("Cycle № "); Serial.print(count + 1); Serial.print(" ");
      //readCurrents();
      readCurrentArray();
      //read_all_currents();
      count++;
    }
    delay(1);
    
    if (count >= cycles_num) {
        Serial.println("Measurement complete!");
        off();
    }
  }

  void on() {
    state = true;
  }
  void off() {
    state = false;
    mode = Mode::IDLE;
    SystemState::free();
  }


  //  Команда для получения состояния и режима измерений
  bool getState() {
    return state;
  }
  uint8_t getMode() {
    return static_cast<uint8_t>(mode);
  }
};