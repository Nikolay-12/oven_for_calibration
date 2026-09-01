#include "Thermodat.h"

//float* temperatures = nullptr;  // Для дальнейшего создания динамического массива температурных стадий
//int temperatures_count = 0;           // Текущее количество элементов в массиве температур

namespace thermoRegulator
{
  bool thermoregState;
  bool reset_flag = false;
  uint32_t THERMODATtmr;
  float stabilization_time = DEFAULT_stabilization_time;
  float holding_time = DEFAULT_holding_time;
  float temperature_tolarance = DEFAULT_temp_tolarance;
  float max_stabilization_time = DEFAULT_max_stab_time;
  float Kp = DEFAULT_KP;
  float Ki = DEFAULT_KI;
  float Kd = DEFAULT_KD;
  float dt = DEFAULT_DT;
  int temperatures_count = 3;
  float temperatures[] = {50.0, 80.0, 100.0};
  int stages_count = 0; // for static array of stages
  float stages[10]; // 10 - max number of stages
  float dT = DEFAULT_TEMP_STEP;
  float initial_temperature = DEFAULT_INITIAL_TEMP;
  float final_temperature = DEFAULT_FINAL_TEMP;
  
  float simulatedTemperature = 25.0;
  MultiStagePIDController* controller = nullptr;
  ThermalModel* heater = nullptr;

  void clear() {
    for (size_t i = 0; i < 10; ++i) {
      stages[i] = 0.0f;
    }
    delete controller;
    controller = nullptr;
    delete heater;
    heater = nullptr;
    stages_count = 0;
  }

  void init() {
    controller = new MultiStagePIDController(stages, stages_count, 
                                  DEFAULT_stabilization_time, DEFAULT_holding_time, DEFAULT_temp_tolarance, DEFAULT_max_stab_time,
                                  DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DEFAULT_DT, 
                                  true);
    /*
    controller = new MultiStagePIDController(temperatures, temperatures_count, 
                                  DEFAULT_stabilization_time, DEFAULT_holding_time, DEFAULT_temp_tolarance, DEFAULT_max_stab_time,
                                  DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DEFAULT_DT, 
                                  true);
    */
    heater = new ThermalModel(25.0,   // Начальная температура
                              0.95,   // Теплоёмкость
                              20.0,  // Тепловое сопротивление
                              10.0,  // Макс. мощность
                              0.15);  // Шум измерения

    Serial.println("========================================");
    Serial.println("   МНОГОСТАДИЙНЫЙ ПИД-РЕГУЛЯТОР");
    Serial.println("========================================");
    Serial.println("Setup complete. Starting control loop...");
    delay(1000);
  }

  void updateStages(const char* data) { // обновление стадий, введенных пользователем через "_"
    resetStages(); // удаление старsых стадий

    stages_count = commandParser::parseStaticFloatArray(data, stages, 10);
    if (stages[0] != 0) {
      Serial.print("Размер полученного float массива: ");
      Serial.println(stages_count);
      Serial.print("Элементы полученного float массива: ");
      for (int i = 0; i < stages_count; i++) {
          Serial.print(stages[i], 1);
          Serial.print("  "); 
      }
      Serial.println(""); 
    } 
    else {
      Serial.println("Ошибка парсинга строки со списком стадий.");
      resetStages();
    }
    reset_flag = true;
    //getData();
  }
  void updateStagesAuto(const float* stages_parameters) { // обновление стадий, введенных пользователем через начало, конец и шаг
    resetStages();
    Serial.print("initial temperature = "); Serial.print(stages_parameters[1]); Serial.print("; final temperature = "); Serial.print(stages_parameters[2]);
    Serial.print("; temperature step = "); Serial.println(stages_parameters[0]);
    if ((stages_parameters[2] - stages_parameters[1]) / stages_parameters[0] > 0.01) {
      dT = stages_parameters[0];
      initial_temperature = stages_parameters[1];
      final_temperature = stages_parameters[2];
      stages_count = (int)((final_temperature - initial_temperature)/dT + 1);
      for (int i = 0; i < stages_count; i++) {
          stages[i] = (float)(initial_temperature + i * dT);
      }
      Serial.print("Размер созданного float массива: ");
      Serial.println(stages_count);
      Serial.print("Элементы созданного float массива: ");
      for (int i = 0; i <= stages_count; i++) {
          Serial.print(stages[i], 1);
          Serial.print("  "); 
      }
      Serial.println(""); 
    }
    else {
      Serial.println("Полученные инструкции для автоматического создания набора температурных этапов некорректны! Перепроверьте их");
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
    Serial.print("PID parameters were changed. New coefs: Kp = "); Serial.print(Kp); Serial.print("; Ki = "); Serial.print(Ki); 
    Serial.print("; Kd = "); Serial.print(Kd); Serial.print("; dt = "); Serial.print(dt); Serial.println(" ms.");
    Serial.println("Thermoregulator settings have been updated! Write 'thermoreg:start;' to start program ...");
    reset_flag = true;
    //getData();
  }

  void updateTimeSettings(const float* time_parameters) { // обновление временных параметров
    stabilization_time = time_parameters[0];
    holding_time = time_parameters[1];
    temperature_tolarance = time_parameters[2];
    max_stabilization_time = time_parameters[3];
    Serial.print("Time parameters were changed. New parameters: stabilization_time = "); Serial.print(stabilization_time); 
    Serial.print(" c; holding_time = "); Serial.print(holding_time); 
    Serial.print(" c; temperature_tolarance = "); Serial.print(temperature_tolarance); 
    Serial.print(" °C; max_stabilization_time = "); Serial.print(max_stabilization_time); Serial.println(" c.");
    Serial.println("Thermoregulator settings have been updated! Write 'thermoreg:start;' to start program ...");
    reset_flag = true;
    //getData();
  }

  void setTemp(const float* new_temp) {
    resetStages();
    stages_count = 1;
    stages[0] = new_temp[0];
    reset_flag = true;
    //getData();
  }

  void thermoregOn() {
    if (reset_flag) {
      controller->reset(stages, stages_count, stabilization_time, holding_time, temperature_tolarance, Kp, Ki, Kd, dt, 
                        max_stabilization_time, true);
      reset_flag = false;
    }
    getData();
    thermoregState = true;
    Serial.println("Thermodat ON");
    THERMODATtmr = millis(); // сброс таймера
  }
  void thermoregOff() {
    thermoregState = false;
    Serial.println("Thermodat OFF");
  }
  
  void thermoregulation() {
    static bool running = true;
    static unsigned long last_update = 0;
    unsigned long now = millis();

    if (now - THERMODATtmr >= (dt * 1000)) {
      THERMODATtmr = millis();
      //float temp = readTemperature();
      float temp = heater->getTemperature();
      float heater_output_precent = 0.0;
      
      running = controller->update(temp, heater_output_precent, dt); // обновление состояния ПИД-регулятора (пока работает 
                                                                  // возвращает true, когда завершается возвращает false)
                                                                  // обновляет heater_output_precent
      //outputPercentToTemp(heater_output_precent, dt); // вызывает функцию тепловой модели для получения новой температуры 
                                                  // ориентируясь на значение выходного управляющего сигнала
      heater->update_temp(heater_output_precent, dt);
      
      if (!running) {  // В конце работы running будет false и это будет означать все этапы завершились
          Serial.println("\n[LOG] Все этапы завершены!");
          //outputPercentToTemp(0.0, dt);
          heater->update_temp(0.0, dt);
          thermoregOff();
      }
    }
    
    delay(1);
  }

  void getData() { 
    Serial.print("Количество стадий: ");
    Serial.println(stages_count);
    //Serial.println(temperatures_count);
    Serial.print("Стадии: ");
    for (int i = 0; i < stages_count; i++) { // temperatures_count
          Serial.print(stages[i], 1);  // temperatures[i]
          Serial.print("  "); 
    }
    Serial.println(""); 
    Serial.print("PID settings: Kp = "); Serial.print(Kp); Serial.print("; Ki = "); Serial.print(Ki); 
    Serial.print("; Kd = "); Serial.print(Kd); Serial.print("; dt = "); Serial.print(dt); Serial.println(" ms.");
    Serial.print("Time settings: stab time = "); Serial.print(stabilization_time); Serial.print(" c ; holding time = "); 
    Serial.print(holding_time); Serial.print(" c ; temp tolarence = "); Serial.print(temperature_tolarance); 
    Serial.print(" C ; max time for stabilization= "); Serial.print(max_stabilization_time); Serial.println(" c.");
  }

  bool getState(){
    return thermoregState;
  }

  // ==================================================
  //  ФУНКЦИИ ДЛЯ ДЛЯ РАБОТЫ С ТЕПЛОВОЙ МОДЕЛЬЮ
  // ==================================================
  float readTemperature() { // температура от тепловой модели
      return heater->getTemperature(); // ТЕСТОВАЯ МОДЕЛЬ
  }

  void outputPercentToTemp(float percent, float DT) { // обновляет температуру, симулированная тепловой моделью
      heater->update_temp(percent, DT); // ТЕСТОВАЯ МОДЕЛЬ
}
}

/*
How to change dynamic array (you want to change stages for example):
void updateStages(int new_size, float* new_arr){
  float* newTemperatures = new float[new_size];
  for (int i = 0; i < new_size; i++) {
      newTemperatures[i] = new_arr[i];
  }

  delete[] stages;
  stages = newTemperatures;

  for (int i = 0; i < new_size; i++) {
      Serial.print(stages[i]);
      Serial.print(" ");
  }
}


for dynamic array of stages:
int stages_count = 0; for dynanic array
float *stages = nullptr;
void clear() {
  delete[] stages;
  stages = nullptr;
  delete controller;
  controller = nullptr;
  delete heater;
  heater = nullptr;
  stages_count = 0;
}
void updateStages(const char* data) { // обновление стадий в памяти контроллера
  ...
  float* stages = commandParser::parseDynamicFloatArray(data, stages_count); // создание нового динамического массива, 
  // параметр stages_count обновляется автоматически в функции extractFloatDynamicArray
*/