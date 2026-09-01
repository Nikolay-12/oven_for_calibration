/*
Тестирование работы с реальными устройствами (термопара MAX6675 и нагреватель с управлением через ШИМ и твердотельное реле)
*/
#include <Command_Parser_2.h>
#include <Arduino.h>
#include "src/Command_Analyzer.h"
#include "src/Thermodat.h"

ParsedCommand cmd;

void setup(void) {
    Serial.begin(9600);
    delay(100);
    Serial.println("Работа начинается...");
    thermoRegulator::init();
    INA219_dop::init();
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
  if (thermoRegulator::getReadingStatus()) {
    thermoRegulator::tempReading();
  }

  if (INA219_dop::getState() && INA219_dop::getMode()) { // чтение показаний INA219 N раз
    INA219_dop::readCurrentSeveralTimes();
  }
  if (INA219_dop::getState() && !INA219_dop::getMode()) { // чтение показаний INA219 до прерывания
    INA219_dop::currentReading();
  }
}
/*
Commands:
thermreg:PIDsettings:0.1_0.01_0.04_0.5; // -> thermreg PIDsettings {5.0, 0.01, 0.04, 0.5} - kp, ki, kd, dt
thermreg:time&tolSettings:10.0_2.0_2.0_20.0; // -> thermreg timeSettings {10.0, 2.0, 2.0, 20.0} - stab time, hold time, tolerance, max hold time
thermreg:tempSettings:20_40_10; // -> thermreg timeSettings {20.0, 80.0, 10.0} - Tmin, Tmax, deltaT
thermreg:tempStages:6_40_50_60_80_90_100; // -> thermreg tempSettings {40.0, 50.0, 60.0, 80.0, 90.0} в float массив записывается определенное количество данных, 
                                              количество записывается первым числом (используется статический массив на 10 элементов)
thermreg:tempStages:2_50_80;
thermreg:tempStagesDyn:5_40_50_60_80_90; // -> thermreg tempSettings {40.0, 50.0, 60.0, 80.0, 90.0} в float массив записывается определенное количество данных, 
                                              количество записывается первым числом (создается динамический массив)
thermreg:timeStagesAuto:40_100_20; // -> thermreg timeSettingsAuto {40.0, 60.0, 80.0, 100.0} в float массив записывается определенное количество температур, 
                                        начиная с 40 и заканчивая 100 с шагом 20 (используется статический массив на 10 элементов)                                       
thermreg:setT:40;
thermreg:changeMode:3; // -> DAC changeMode {3} - меняет режим работы терморегулятора 
(0 - холостой ход;
 1 - изменение температуры исходя из набора этапов 
 2 - постепенное повышение температуры Tmin до Tmax с шагом dT;
 3 - постепенное снижение температуры с Tmax до Tmin с шагом dT;)
thermreg:start;
thermreg:stop;

thermreg:startTempReading;
thermreg:stopTempReading;
thermreg:readAllTemp;
thermreg:readAvrTemp;
thermreg:stopTempsReading;

INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {10, 1.0} - количество циклов измерений и перерыв между ними
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219 с опредленным периодом
INA219:reading; // -> INA219 reading - считывает значения с INA219 с опредленным периодом до прерывания
INA219:stop; // -> INA219 stop - прерывание считывания
*/