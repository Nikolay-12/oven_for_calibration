#include <Command_Parser_2.h>
#include "main_for_DAC0808.h"
#include "main_for_INA219.h"
#include "74HC595_register.h"
#include "auxiliary_file.h"
#include "System_State.h"


namespace DAC0808
{
  bool state;
  uint8_t mode = 0; // режимы работы ЦАП

  int stages_count = 0; // реальное количество стадий (до 10)
  float stages[10]; // массив стадий
  float Vmin = DEFAULT_VOLTAGE_MIN;
  float Vmax = DEFAULT_VOLTAGE_MAX;
  float deltaV = DEFAULT_VOLTAGE_STEP;
  float current_voltage = DEFAULT_VOLTAGE_MIN;
  uint8_t MAX_VALUE = MAX_VALUE_8BIT;
  int min_level = 0; // минимальное значение сигнала ЦАП
  int max_level = MAX_VALUE; // максимальное значение сигнала ЦАП
  int current_level = 0; // текущее значение сигнала ЦАП
  int level_step = 1; // шаг увеличения/уменьшение уровня сигнала ЦАП
  uint8_t cycles_num = 10;
  float measurement_time = 1.0; // время между измерениями
  


  bool reset_flag = false;
  uint32_t tmr;
  float dt_s = 0.1;
  float holding_time = DEFAULT_holding_time;

  MultiStageDACcontroller* DACcontroller = nullptr;
  ShiftRegister74HC595 DAC_reg(DAC_HC595_DS_PIN, DAC_HC595_ST_PIN, DAC_HC595_SH_PIN);

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
                                  setOutputLevel, INA219controller::readCurrentArray,
                                  cycles_num, measurement_time, currentReadings, 20,
                                  measurementProcessing::processMeasurementsIV);
    
    DAC_reg.init();
    //pinMode(DAC_OUTPUT_PIN, INPUT);
    
    Serial.println("Многостадийный регулятор для ЦАП на базе DAC0808 в связке со сдвиговым регистром 74HC595 инициализирован.");
    delay(500);
  }

  //  Команды для обновления/удаления стадий по напряжению ЦАПа
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
    SystemState::free();
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

  // ==============================================================
  //  ФУНКЦИЯ ДЛЯ НЕПОСРЕДСТВЕННОЙ РАБОТЫ С ЦАП (8-битный DAC0808)
  // ==============================================================
  void setOutputLevel(size_t level, bool save_to_eeprom) // устанавливает уровень сигнала на выходе с ЦАП
  {
      DAC_reg.write(level, true); // управление ЦАПом происходит через сдвиговый регистр 74HC595
      delayMicroseconds(100);
  }
};