/*
Тестирование работы с реальными устройствами (цифро-аналоговый преобразователь MCP4725 с управлением через I2C)
*/
#include <Command_Parser_2.h>
#include <Arduino.h>
#include "src/Command_Analyzer.h"
#include "src/main_for_DAC_MCP4725.h"
#include "src/main_for_INA219.h"

ParsedCommand cmd;
namespace DAC = DAC_MCP4725;

void setup(void) {
    Serial.begin(9600);
    delay(100);
    DAC::init();
    INA219_dop::init();
    Serial.println("DAC start working...");
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

  if (DAC::getState()) {
    DAC::inProgress();
  }
}
/*
Commands:
DAC:settings:0_3_1_20; // -> DAC settings {0.0, 5.0, 1.0, 20.0} - Vmin, Vmax, dV, holding time
DAC:setHoldingTime:10; // -> DAC settings {10.0} - holding time
DAC:voltageSettings:4_1_2_2.5_3; // -> DAC settings {1.0, 2.0, 2.5, 3.0}
DAC:voltageSettingAuto:4_0_3; // -> нужно ли?
DAC:setV:3; // -> DAC setV {1.0}
DAC:changeMode:3; // -> DAC changeMode {3} - меняет режим работы ЦАП 
(0 - холостой ход;
 1 - изменение напряжения исходя из набора этапов 
 2 - постепенное повышение напряжения с Vmin до Vmax с шагом dV;
 3 - постепенное снижение напряжения с Vmax до Vmin с шагом dV;)
DAC:start; // -> DAC start
DAC:stop; // -> DAC stop

INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {10} - number of measurement cycles
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219 с опредленным периодом
INA219:reading; // -> INA219 reading - считывает значения с INA219 с опредленным периодом до прерывания
INA219:stop; // -> INA219 stop - прерывание считывания
*/