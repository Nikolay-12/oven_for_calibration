#include "project_config.h"
#include "Thermodat.h"
#include "auxiliary_file.h"
#include "thermocouples.h"


// Глобальные буферы температур и токов
float tempReadings[20];
float currentReadings[20];

bool i2cInitialized = false;
bool calibrateMAX6675 = true;
ShiftRegister74HC595 reg(HC595_DS_PIN, HC595_ST_PIN, HC595_SH_PIN);

namespace thermoRegulator
{
  bool state;
  uint8_t mode = 0; // режимы работы терморегулятора
  bool reset_flag = false;
  bool isReading = false;
  uint32_t tmr;
  float stabilization_time = DEFAULT_stabilization_time;
  float holding_time = DEFAULT_holding_time;
  float temperature_tolarance = DEFAULT_temp_tolarance;
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
                                  DEFAULT_stabilization_time, DEFAULT_holding_time, DEFAULT_temp_tolarance, DEFAULT_max_stab_time,
                                  DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DEFAULT_DT, 
                                  true,
                                  readTemperature, readAllTempAndReturnThem, INA219_dop::readINAcurrent, 
                                  cycles_num, measurement_time, tempReadings, currentReadings, 20,
                                  measurementProcessing::processMeasurementsIT);

    pinMode(RELAY_PIN, OUTPUT);
    thermocouples::init(&reg);
    Serial.println("Thermocouples MAX6675 were initialized...");

    Serial.println("========================================");
    Serial.println("   МНОГОСТАДИЙНЫЙ ПИД-РЕГУЛЯТОР");
    Serial.println("========================================");
    Serial.println("Setup complete. Starting control loop...");
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
      THERMALcontroller->temp_settings_reset(mode, Tmin, Tmax, deltaT);
      mode = 2;
      Serial.print("Temperature parameters were changed. New parameters: Tmin = "); Serial.print(Tmin); Serial.print(" °C; Tmax = "); 
      Serial.print(Tmax); Serial.print(" °C; deltaT = "); Serial.print(deltaT); Serial.println(" °C.");
      Serial.println("Write 'thermreg:start;' to start program ...");
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

  void updateTimeSettings(const float* time_parameters) { // обновление временных параметров
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
  }

  void updateMeasurementSettings(const int cycles, const float period) { // обновление базовых настроек ЦАП
    cycles_num = cycles;
    measurement_time = period;
    THERMALcontroller->measurement_reset(cycles_num, measurement_time);
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
    if (new_mode == 0 || new_mode == 1 || new_mode == 2 || new_mode == 3 || new_mode == 4 || new_mode == 5) {
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
        case 4:
          Serial.println(" (reading average temperature mode)");
          break;
      case 5:
          Serial.println(" (reading all temperatures mode)");
          break;
      }
    }
    else {
      Serial.println("There is no such mode!");
      Serial.println("Idle mode was chosen.");
      mode = 0;
    }
  }

  void on() {
    if (mode != 0) {
      THERMALcontroller->scriptStarting(mode);
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
    Serial.println("Start reading...");
    isReading = true;
  }
  void stopTempReading() {
    Serial.println("Reading stoped.");
    isReading = false;
  }
  void tempReading() {
    if (!isReading) return;
    static uint32_t last_temp_read = millis();
    if (millis() - last_temp_read >= 1000) {
      last_temp_read = millis();
      if (mode == 4) { // Режим измерения средней температуры
        readAllTempAndReturnAverage();
      }
      if (mode == 5) { // Режим измерения всех температур
        readAllTempAndReturnThem();
      }
      Serial.println("");
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
    for (int i = 0; i < stages_count; i++) { 
      Serial.print(stages[i], 1); 
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
  float readTemperature() {  // Чтение усредненной температуры с восьми термопар
    thermocouples::read_all_temp_once(calibrateMAX6675);
    float temp = thermocouples::get_average_temp_celsius();
    return temp;
  }

  void setHeaterOutput(float percent) { // подает на нагреватель ШИМ сигнал
    int pwm = map(percent, 0, 100, 0, 255);
    analogWrite(RELAY_PIN, pwm);
  }


  void readAvrTemp() { 
    mode = 4;
    Serial.println("Reading average temperature mode was set"); 
  }
  void readAllTemps() { 
    mode = 5;
    Serial.println("Reading all temperatures (25) mode was set"); 
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
    thermoRegulator::updateMeasurementSettings(cycles_num, period_s);
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



void initI2C(uint8_t SDA_PIN, uint8_t SCL_PIN) {
  if (!i2cInitialized) { // Инициализация шины I2C, если она ещё не инициализирована
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    i2cInitialized = true;
    Serial.println("I2C инициализирована.");
  }
}