/*
Тестирование работы с массивом из 4-х INA219 с управлением через main_controller
*/
#include <Command_Parser_2.h>
#include <Arduino.h>
#include "src/Command_Analyzer.h"
#include "src/main_controller.h"

ParsedCommand cmd;

void setup(void) {
    Serial.begin(9600);
    delay(100);
    MainController::init();
}

void loop(void) {
  static String receivedData=""; 
  if (Serial.available() > 0) {
    receivedData = Serial.readString();
    receivedData.trim();
    Serial.flush();
    char inputBuffer[64]; // буфер должен быть на 1 больше длины строки!
    if (sizeof(inputBuffer) < receivedData.length()) {
      Serial.println("Слишком большая строка!");
    }
    else {
      receivedData.toCharArray(inputBuffer, sizeof(inputBuffer)); // Преобразование String в массив
      commandParser::clear(&cmd); // очищает данные предыдущей команды
      
      commandParser::analyzer(inputBuffer, &cmd); // разбивает строку на Device, Key и оставшуюся строку-инструкцию
      commandAnalyzer::analyzeCommand(&cmd); // исходя из Device, Key и оставшейся строки-инструкции передает данные соответствующим блокам кода
    }
  }

  if (MainController::getState()) {
    MainController::inProgress(); // включение основного обработчика
  }
}
/*
Commands:
mainController:setMainMode:1;
(0 - холостой ход;
 1 - режим измерения I(T);
 2 - режим измерения I(V))
mainController:stop;

INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {20} {1.0} - количество циклов измерений (20) с периодом в 1.0 сек
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с датчиков INA219 с определенным периодом
INA219:reading; // -> INA219 reading - считывает значения с датчиков INA219 с определенным периодом до прерывания
INA219:stop; // -> INA219 stop - останавливает измерения с датчиков
*/