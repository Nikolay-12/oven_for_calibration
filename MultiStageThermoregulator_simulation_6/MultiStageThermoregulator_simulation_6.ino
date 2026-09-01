/*
Тестирование работы библиотеки Command_Parser_2 для парсинга введенных в порт данных без устройств с симуляцией нагрева
*/
#include <Command_Parser_2.h>
#include <Arduino.h>
#include "src/Command_Analyzer.h"
#include "src/Thermodat.h"

ParsedCommand cmd;

void setup(void) {
    Serial.begin(9600);
    delay(1000);
    Serial.println("Работа начинается...");
    thermoRegulator::init();
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
      commandParser::clear(&cmd);
      
      commandParser::analyzer(inputBuffer, &cmd); // разбивает строку на Device, Key и оставшуюся строку-инструкцию
      commandAnalyzer::analyzeCommand(&cmd); // исходя из Device, Key и оставшейся строки-инструкции выполняет команду
    }
  }

  if (thermoRegulator::getState()) {
    thermoRegulator::thermoregulation();
  }
}
/*
Commands:
thermreg:PIDsettings:5.0_0.01_0.04_0.5; // -> thermreg PIDsettings {5.0, 0.01, 0.04, 0.5} в float массив записывается фиксированное количество данных (до 5)
thermreg:timeSettings:10_5.0_2.0_100.0; // -> thermreg timeSettings {20.0, 10.0, 1.0, 180.0} в float массив записывается фиксированное количество данных (до 5)
thermreg:tempSettings:6_40_50_60_80_90_100; // -> thermreg tempSettings {40.0, 50.0, 60.0, 80.0, 90.0} в float массив записывается определенное количество данных, 
                                              количество записывается первым числом (используется статический массив на 10 элементов)
thermreg:tempSettingsDyn:5_40_50_60_80_90; // -> thermreg tempSettings {40.0, 50.0, 60.0, 80.0, 90.0} в float массив записывается определенное количество данных, 
                                              количество записывается первым числом (создается динамический массив)
thermreg:tempSettingsAuto:20_40_100; // -> thermreg timeSettingsAuto {40.0, 60.0, 80.0, 100.0} в float массив записывается определенное количество температур, 
                                        начиная с 40 и заканчивая 100 с шагом 20 (используется статический массив на 10 элементов)                                       
thermreg:setT:40;
thermreg:start;
thermreg:stop;
*/