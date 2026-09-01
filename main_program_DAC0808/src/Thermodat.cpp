#include <Command_Parser_2.h>
#include "project_config.h"
#include "Thermodat.h"
#include "main_for_DAC0808.h"
#include "main_for_INA219.h"
#include "auxiliary_file.h"
#include "thermocouples.h"
#include "System_State.h"

float tempReadings[20] = {0};
float currentReadings[20] = {0};
bool calibrateMAX6675 = true;
ShiftRegister74HC595 reg(HC595_DS_PIN, HC595_ST_PIN, HC595_SH_PIN);

namespace thermoRegulator
{
  bool state;
  enum Mode  {IDLE = 0, 
              MULTI_STAGE = 1, // Многостадийный пользовательский режим
              TEMPERATURE_INCREASING = 2, // Режим поэтапного увеличения напряжения с Vmin до Vmax
              TEMPERATURE_DECREASING = 3, // Режим поэтапного уменьшения напряжения с Vmax до Vmin
              READING_AVERAGE_TEMP = 4, // Режим измерения средней температуры
              READING_ALL_TEMPS = 5}; // Режим измерения всех температур
  //Mode mode = Mode::IDLE;
  uint8_t mode = 0; // режимы работы терморегулятора
  bool reset_flag = false;
  bool isReading = false;
  uint32_t tmr;
  float stabilization_time = DEFAULT_stabilization_time;
  const float DEFAULT_holding_time = 10.0;
  float holding_time = DEFAULT_holding_time;
  float temperature_tolarance = DEFAULT_temp_tolerance;
  float max_stabilization_time = DEFAULT_max_stab_time;
  float Kp = DEFAULT_KP;
  float Ki = DEFAULT_KI;
  float Kd = DEFAULT_KD;
  float dt = DEFAULT_DT;
  int stages_count = 0; // for static array of stages
  float stages[10]; // 10 - max number of stages
  float deltaT = DEFAULT_TEMPERATURE_STEP;
  float Tmin = DEFAULT_TEMPERATURE_MIN;
  float Tmax = DEFAULT_TEMPERATURE_MAX;
  uint8_t cycles_num = 10;
  float measurement_time = 1.0; // время между измерениями
  
  MultiStagePIDController* THERMALcontroller = nullptr;
  GyverMAX6675 <SPI_CLK_PIN, SPI_MISO_PIN, SPI_CS_PIN> sensor;
  
  float saved_temp = 0.0f;
  uint32_t last_temp_reading;

  void clear() {
    for (size_t i = 0; i < 10; ++i) {
      stages[i] = 0.0f;
    }
    delete THERMALcontroller;
    THERMALcontroller = nullptr;
    stages_count = 0;
  }

  void init() {
    THERMALcontroller = new MultiStagePIDController(mode, stages, stages_count, 
                                  DEFAULT_stabilization_time, DEFAULT_holding_time, DEFAULT_temp_tolerance, DEFAULT_max_stab_time,
                                  DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DEFAULT_DT, 
                                  true,
                                  readTemperature, readAllTempAndReturnThem, INA219controller::readCurrentArray, 
                                  cycles_num, measurement_time, tempReadings, currentReadings, 20,
                                  measurementProcessing::processMeasurementsIT);

    pinMode(OVEN_PWM_PIN, OUTPUT);
    thermocouples::init(&reg);
    Serial.println("Массив из 25 термопар инициализирован.");

    Serial.println("Многостадийный ПИД-регулятор инициализирован.");
    saved_temp = readTemperature();
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
      THERMALcontroller->stages_settings_reset(stages, stages_count);
    } 
    else {
      Serial.println("Ошибка парсинга строки со списком стадий.");
      resetStages();
    }
    mode = 1;
    delay(100);
  }
  void updateTempSettings(const float* param) { // обновление стадий, введенных пользователем через начало, конец и шаг
    resetStages();
    Tmin = param[0];
    Tmax = param[1];
    deltaT = param[2];
    if ((Tmin < 0) || (Tmax > 700) || (Tmin >= Tmax) || (deltaT < 1) || (!isDivisible(Tmax-Tmin, deltaT))){
      Serial.println("There is problem this the settings!");
      Serial.println("Default settings have been applied.");
      Tmin = DEFAULT_TEMPERATURE_MIN;
      Tmax = DEFAULT_TEMPERATURE_MAX;
      deltaT = DEFAULT_TEMPERATURE_STEP;
      stages_count = 0;
    }
    else {
      stages_count = (int)((Tmax - Tmin)/deltaT);
      mode = 2;
      THERMALcontroller->temp_settings_reset(mode, Tmin, Tmax, deltaT);
      Serial.print("Температурные настройки были изменены. Новые параметры: Tmin = "); Serial.print(Tmin); Serial.print(" °C; Tmax = "); 
      Serial.print(Tmax); Serial.print(" °C; deltaT = "); Serial.print(deltaT); Serial.println(" °C.");
      Serial.println("По умолчанию установлен режим TEMPERATURE_INCREASING.");
      Serial.println("Введите команду 'thermreg:start;' для запуска терморегулятора...");
      delay(100);
    }
  }
  void resetStages() {
    for (size_t i = 0; i < 10; ++i) {
      stages[i] = 0.0f;
    }
    stages_count = 0;
  }

  void updatePIDcoef(const float* PID_parameters) {  // обновление параметров ПИД-регулятора
    Kp = PID_parameters[0];
    Ki = PID_parameters[1];
    Kd = PID_parameters[2];
    dt = PID_parameters[3];
    THERMALcontroller->PID_settings_reset(Kp, Ki, Kd, dt);
    Serial.print("PID parameters were changed. New coefs: Kp = "); Serial.print(Kp); Serial.print("; Ki = "); Serial.print(Ki); 
    Serial.print("; Kd = "); Serial.print(Kd); Serial.print("; dt = "); Serial.print(dt); Serial.println(" s.");
    Serial.println("Thermoregulator settings have been updated! Write 'thermreg:start;' to start program ...");
    delay(100);
  }

  void updateTimeTolSettings(const float* time_parameters) { // обновление временных параметров
    if ((time_parameters[0] > 0) && (time_parameters[1] > 0) && (time_parameters[2] > 0) && (time_parameters[3] > 0)) {
      stabilization_time = time_parameters[0];
      holding_time = time_parameters[1];
      temperature_tolarance = time_parameters[2];
      max_stabilization_time = time_parameters[3];
      THERMALcontroller->time_and_tol_settings_reset(stabilization_time, holding_time, temperature_tolarance, max_stabilization_time);
      delay(100);
      Serial.print("Time parameters were changed. New parameters: stabilization_time = "); Serial.print(stabilization_time); 
      Serial.print(" c; holding_time = "); Serial.print(holding_time); 
      Serial.print(" c; temperature_tolarance = "); Serial.print(temperature_tolarance); 
      Serial.print(" °C; max_stabilization_time = "); Serial.print(max_stabilization_time); Serial.println(" c.");
      Serial.println("Thermoregulator settings have been updated! Write 'thermreg:start;' to start program ...");
      THERMALcontroller->measurement_reset(cycles_num, measurement_time);
    }
    else {
      Serial.println("Некоторые параметры заданы неверно. Проверьте настройки и повторите попытку.");
    }
  }

  void setTempPoint(const float new_tempPoint) { // устанавливает определенное напряжение на выходе с ЦАП
    resetStages();
    float T = new_tempPoint;
    if ((T < 0) || (T > 700)) { 
      Serial.println("The received temperature is out of range!");
      Serial.println("The temperature is set to 20 °C.");
      mode = 0;
    }
    else {
      stages_count = 1;
      stages[0] = new_tempPoint;
      THERMALcontroller->stages_settings_reset(stages, stages_count);
      mode = 1;
      Serial.print("The temperature of "); Serial.print(new_tempPoint); Serial.println(" °C is ready to set.");
      Serial.println("Write 'thermreg:start;' to start program ...");
    }
  }

  void changeMode(const int new_mode) {
    if (new_mode == 0 || new_mode == 1 || new_mode == 2 || new_mode == 3) {
      mode = new_mode;
      Serial.print("New mode = "); Serial.print(mode);
      switch (mode) {
        case 0:
          Serial.println(" (idle mode)");
          break;
        case 1:
          Serial.println(" (multi-stage mode)");
          break;
        case 2:
          Serial.println(" (temperature increasing mode)");
          break;
        case 3:
          Serial.println(" (temperature decreasing mode)");
          break;
      }
    }
    else {
      Serial.println("There is no such mode!");
      Serial.println("Idle mode was chosen.");
      mode = 0;
    }
  }

  void updateMeasurementSettings(const int cycles, const float period) { // обновление базовых настроек терморегулятора
    cycles_num = cycles;
    measurement_time = period;
    THERMALcontroller->measurement_reset(cycles_num, measurement_time);
  }

  void on() {
    if (mode != 0) {
      THERMALcontroller->scriptStarting(mode);
      DAC0808::setOutputLevel(VOLTAGE_MIN_FOR_IT_MODE, false);
      getData();
      state = true;
      Serial.println("Thermodat ON");
      tmr = millis(); // сброс таймера
      last_temp_reading = millis();
    }
    else if (mode == 0) {
      Serial.println("The operating mode was not selected!");
    }
  }
  void off() {
    state = false;
    Serial.println("Thermodat OFF");
    DAC0808::setOutputLevel(0, false);
    SystemState::free();
  }
  
  void thermoregulation() { // последовательность: определение температуры -> расчет необходимого управляющего сиганала -> отправка ПИД-регулятору
    static bool running = true;
    unsigned long now = millis();

    if (now - tmr >= (dt * 1000)) {
      tmr = now;

      if (now - last_temp_reading >= (MAX6675_READING_TIMEOUT * 1000)) {
        last_temp_reading = now;
        saved_temp = readTemperature(); // считывает температуру с термопары MAX6675
      }

      float heater_output_precent = 0.0;
      running = THERMALcontroller->update(saved_temp, heater_output_precent, dt); // обновление состояния ПИД-регулятора (пока работает 
                                                                  // возвращает true, когда завершается возвращает false)
                                                                  // обновляет heater_output_precent
      setHeaterOutput(heater_output_precent); // отправляет рассчитанный управляющий сигнал на нагреватель (это приведет к разогреву/охлаждению, что изменит temp)

      if (!running) {  // В конце работы running будет false и это будет означать все этапы завершились
          Serial.println("\n[LOG] Все этапы завершены!");
          setHeaterOutput(0.0);
          off();
      }
    }
    delay(1);
  }

  void startTempReading() {
    if ((mode == 4) || (mode == 5)) { 
        Serial.println("Start reading...");
        isReading = true;
      }
    else {
        Serial.print("Исполнение режима "); Serial.print(mode); Serial.println(" не предусмотрено!"); 
        isReading = false;
    }
    
  }
  void stopTempReading() {
    Serial.println("Reading stoped.");
    isReading = false;
    SystemState::free();
  }
  void tempReading() {
    if (!isReading) return;
    static uint32_t last_temp_read = millis();
    if (millis() - last_temp_read >= 1000) {
      last_temp_read = millis();
      if (mode == 4) { // Режим измерения средней температуры
        readAllTempAndReturnAverage();
      }
      else if (mode == 5) { // Режим измерения всех температур
        readAllTempAndReturnThem();
        Serial.println("");
      }
    }
  }
  bool getReadingStatus() {return isReading;}

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
          Serial.println(" (Temperature increasing mode)");
          break;
      case 3:
          Serial.println(" (Temperature decreasing mode)");
          break;
      case 4:
          Serial.println(" (Reading average temperature mode)");
          break;
      case 5:
          Serial.println(" (Reading all temperatures mode)");
          break;
    }
    Serial.print("Количество стадий: ");
    Serial.println(stages_count);
    Serial.print("Стадии: ");
    for (int i = 0; i < stages_count; i++) { // temperatures_count
      Serial.print(stages[i], 1);  // temperatures[i]
      Serial.print("  "); 
    }
    Serial.println(""); 
    Serial.print("PID settings: Kp = "); Serial.print(Kp); Serial.print("; Ki = "); Serial.print(Ki); 
    Serial.print("; Kd = "); Serial.print(Kd); Serial.print("; dt = "); Serial.print(dt); Serial.println(" s.");
    Serial.print("Time settings: stab time = "); Serial.print(stabilization_time); Serial.print(" s ; holding time = "); 
    Serial.print(holding_time); Serial.print(" s ; temp tolarence = "); Serial.print(temperature_tolarance); 
    Serial.print(" °C ; max time for stabilization = "); Serial.print(max_stabilization_time); Serial.println(" s.");
  }
  bool getState(){
    return state;
  }
  uint8_t getMode() {
    return mode;
  }

  // ====================================================
  //  ФУНКЦИИ ДЛЯ ДЛЯ РАБОТЫ С ТЕРМОПАРОЙ И НАГРЕВАТЕЛЕМ
  // ====================================================
  float readTemperature() {  // Чтение температуры с термопары
    thermocouples::read_all_temp_once(calibrateMAX6675);
    float temp = thermocouples::get_average_temp_celsius();
    return temp;
  }

  void setHeaterOutput(float percent) { // подает на нагреватель ШИМ сигнал
    int pwm = map(percent, 0, 100, 0, 255);
    analogWrite(OVEN_PWM_PIN, pwm);
  }

  void readAvrTemp() { 
    mode = 4;
    Serial.println("Reading average temperature mode was set"); 
  }
  void readAllTemps() { 
    mode = 5;
    Serial.println("Reading all temperatures (25 thermocouples) mode was set"); 
  }
  
  void readAllTempAndReturnAverage() {
    thermocouples::read_all_temp_once(calibrateMAX6675);
    Serial.print("Average temperature: "); Serial.println(thermocouples::get_average_temp_celsius()); 
  }
  void readAllTempAndReturnThem() {
    thermocouples::read_all_temp_once(calibrateMAX6675);
    thermocouples::send_temp_array(calibrateMAX6675);
    //delay(250);
  }
};