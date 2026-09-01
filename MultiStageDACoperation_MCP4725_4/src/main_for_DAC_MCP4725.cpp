#include "main_for_DAC_MCP4725.h"
#include "main_for_INA219.h"
#include "auxiliary_file.h"

// Глобальные буфер токов
float currentReadings[20];

/*
struct MeasurementConfig { // структура для хранения конфигурации измерений
    uint8_t measurement_count = 5;
    float measurement_interval_s = 1.0f;
};
MeasurementConfig measureConfig;
*/

namespace DAC_MCP4725
{
  bool state;
  uint8_t mode = 0; // режимы работы ЦАП

  int stages_count = 0; // реальное количество стадий (до 10)
  float stages[10]; // массив стадий
  float Vmin = DEFAULT_VOLTAGE_MIN;
  float Vmax = DEFAULT_VOLTAGE_MAX;
  float deltaV = DEFAULT_VOLTAGE_STEP;
  float current_voltage = DEFAULT_VOLTAGE_MIN;
  int min_level = 0; // минимальное значение сигнала ЦАП
  int max_level = 4095; // максимальное значение сигнала ЦАП
  int current_level = 0; // текущее значение сигнала ЦАП
  int level_step = 1; // шаг увеличения/уменьшение уровня сигнала ЦАП
  uint8_t cycles_num = 10;
  float measurement_time = 1.0; // время между измерениями
  


  bool reset_flag = false;
  uint32_t tmr;
  float dt_s = 0.1;
  float holding_time = DEFAULT_holding_time;

  MultiStageDACcontroller* DACcontroller = nullptr;


  void clear() {
    for (size_t i = 0; i < 10; ++i) {
      stages[i] = 0.0f;
    }
    delete DACcontroller;
    DACcontroller = nullptr;
    stages_count = 0;
  }

  void init() {
    DACcontroller = new MultiStageDACcontroller(mode, stages, stages_count, DEFAULT_holding_time, 
                                  true,
                                  setOutputLevel, INA219_dop::readINAcurrent, 
                                  cycles_num, measurement_time, currentReadings, 20,
                                  measurementProcessing::processMeasurementsIV);
    
    initI2C(I2C_SDA_PIN, I2C_SCL_PIN);
    pinMode(output_DAC_pin, INPUT);
    
    Serial.println("========================================");
    Serial.println("   МНОГОСТАДИЙНЫЙ ЦАП на базе MCP4725");
    Serial.println("========================================");
    Serial.println("Setup complete. Starting control loop...");
    delay(500);
  }

  void updateStages(const char* data) { // обновление стадий в памяти контроллера
    resetStages(); // удаление старого динамического массива

    stages_count = commandParser::parseStaticFloatArray(data, stages, 10);
    if (stages_count != 0) {
      Serial.print("Размер полученного float массива: ");
      Serial.println(stages_count);
      Serial.print("Элементы полученного float массива: ");
      for (int i = 0; i < stages_count; i++) {
          Serial.print(stages[i], 1);
          Serial.print("  "); 
      }
      Serial.println(""); 
      DACcontroller->reset_stages(stages, stages_count);
    } 
    else {
      Serial.println("Ошибка парсинга строки со списком стадий.");
      resetStages();
    }
    mode = 1;
  }
  void updateStagesAuto(const float* STAGESparam) { // need?
    // ???
  } 
  void resetStages() {
    for (size_t i = 0; i < 10; ++i) {
      stages[i] = 0.0f;
    }
    stages_count = 0;
  }

  void updateSettings(const float* param) { // обновление базовых настроек ЦАП
    Vmin = param[0];
    Vmax = param[1];
    deltaV = param[2];
    holding_time = param[3];
    if ((Vmin < 0) || (Vmax > 3.3) || (Vmin >= Vmax) || (deltaV < 0.01) || (!isDivisible(Vmax-Vmin, deltaV)) || (holding_time <= 0)) { // for ESP32
      Serial.println("There is problem this the settings!");
      Serial.println("Default settings have been applied.");
      Vmin = DEFAULT_VOLTAGE_MIN;
      Vmax = DEFAULT_VOLTAGE_MAX;
      deltaV = DEFAULT_VOLTAGE_STEP;
      holding_time = DEFAULT_holding_time;
    }
    min_level = round(Vmin / Vref * MAX_VALUE);
    max_level = round(Vmax / Vref * MAX_VALUE);
    current_level = min_level;
    level_step = round(deltaV / Vref * MAX_VALUE);
    DACcontroller->reset_level_settings(2, min_level, max_level, level_step); // 2 -> mode 
    DACcontroller->reset_time_settings(holding_time);
    mode = 2;
    
    Serial.print("DAC parameters were changed. New parameters: Vmin = "); Serial.print(Vmin); Serial.print(" V; Vmax = "); 
    Serial.print(Vmax); Serial.print(" V; deltaV = "); Serial.print(deltaV); Serial.print(" V; holding time = "); 
    Serial.print(holding_time); Serial.println(" s.");
    Serial.println("Write 'DAC:start;' to start program ...");
    delay(500);
  }
  void updateTimeSettings(const float new_hold_time) {
    holding_time = new_hold_time;
    DACcontroller->reset_time_settings(holding_time);
  }
  void updateMeasurementSettings(const int cycles, const float period) { // обновление базовых настроек ЦАП
    cycles_num = cycles;
    measurement_time = period;
    DACcontroller->measurement_reset(cycles_num, measurement_time);
  }
  void setVoltagePoint(const float new_voltPoint) { // устанавливает определенное напряжение на выходе с ЦАП
    resetStages();
    float V = new_voltPoint;
    if ((V < 0) || (V > 3.3)) { // for ESP32
      Serial.println("The received voltage is out of range!");
      Serial.println("The voltage is set to 0 V.");
      mode = 0;
    }
    else {
      stages_count = 1;
      stages[0] = new_voltPoint;
      DACcontroller->reset_stages(stages, stages_count);
      mode = 1;
      Serial.print("The voltage of "); Serial.print(new_voltPoint); Serial.println(" V is ready to set.");
      Serial.println("Write 'DAC:start;' to start program ...");
    }
  } 

  void changeMode(const int new_mode) {
    if (new_mode == 0 || new_mode == 1 || new_mode == 2 || new_mode == 3) {
      mode = new_mode;
      Serial.print("New mode = "); Serial.println(mode);
    }
    else {
      Serial.println("There is no such mode!");
      mode = 0;
    }
  }

  void inProgress() { // основная функция, которавя выполняется в loop()
    static bool running = true;
    static unsigned long last_update = 0;
    unsigned long now = millis();

    if (now - tmr >= (dt_s * 1000)) {
      tmr = millis();
      
      running = DACcontroller->update(mode, &current_level); // обновление состояния ЦАП

      if (!running) {  // В конце работы running будет false и это будет означать все этапы завершились
          Serial.println("\n[LOG] Все этапы завершены!");
          setOutputLevel(0, false);
          off();
      }
    }
    delay(1);
  } 

  void on() { // запускает работу ЦАП
    getData();
    Serial.println("DAC ON");
    tmr = millis(); // сброс таймера
    DACcontroller->scriptStarting(mode, &current_level);
    state = true;
  } 
  void off() { // прерывает работу ЦАП
    state = false;
    Serial.println("DAC OFF");
  } 


  int getLevel(){
    return current_level;
  }
  void getData() { 
    Serial.print("Режим работы: ");
    Serial.print(mode);
    switch (mode) {
      case 0:
          Serial.println(" (Idle mode)");
          break;
      case 1:
          Serial.println(" (Multi-stage mode)");
          break;
      case 2:
          Serial.println(" (Voltage increasing mode)");
          break;
      case 3:
          Serial.println(" (Voltage decreasing mode)");
          break;
    }
    Serial.print("Количество стадий: ");
    Serial.println(stages_count);
    Serial.print("Стадии: ");
    for (int i = 0; i < stages_count; i++) { 
          Serial.print(stages[i], 1);  
          Serial.print("  "); 
    }
    Serial.println(""); 
    Serial.print("DAC parameters: Vmin = "); Serial.print(Vmin); Serial.print(" V; Vmax = "); Serial.print(Vmax); 
    Serial.print(" V; deltaV = "); Serial.print(deltaV); Serial.print(" V; holding time = "); 
    Serial.print(holding_time); Serial.println(" s.");
    Serial.print("Current voltage: "); Serial.print(current_voltage); Serial.println(" V");
  }
  bool getState(){
    return state;
  }
  uint8_t getMode(){
    return mode;
  }

  // ====================================================
  //  ФУНКЦИЯ ДЛЯ НЕПОСРЕДСТВЕННОЙ РАБОТЫ С ЦАП
  // ====================================================
  void setOutputLevel(size_t level, bool save_to_eeprom) // устанавливает уровень сигнала на выходе с ЦАП
  {
      uint8_t cmd[3]; 
  
      if (save_to_eeprom == true) {
          cmd[0] = 0x60;
      } else {
          cmd[0] = 0x40;
      }
      
      // Формируем данные для передачи на ЦАП
      cmd[1] = (level / 16);        // Старшие биты данных
      cmd[2] = (level % 16) << 4;   // Младшие биты данных 
  
      Wire.beginTransmission(DAC_ADDR); // Начинаем общение с датчиком
      Wire.write(cmd, 3);           // Передаем команду из 3 байт
      Wire.endTransmission();       // Завершаем передачу данных
      delayMicroseconds(100);
  }
};