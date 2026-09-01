/*
Тестирование работы с реальными устройствами (терморегултор с управлением через ШИМ, ЦАП MCP4725 и INA219 
с управлением через I2C, а также управление MOSFET-транзистором для переключения между состояниями в механических реле)
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

thermreg:PIDsettings:0.1_0.01_0.04_0.5; // -> thermreg PIDsettings {5.0, 0.01, 0.04, 0.5} - kp, ki, kd, dt
thermreg:time&tolSettings:10.0_2.0_2.0_20.0; // -> thermreg timeSettings {10.0, 2.0, 2.0, 20.0} - stab time, hold time, tolerance, max hold time
thermreg:tempSettings:20_40_10; // -> thermreg timeSettings {20.0, 80.0, 10.0} - Tmin, Tmax, deltaT
thermreg:tempStages:6_40_50_60_80_90_100; // -> thermreg tempSettings {40.0, 50.0, 60.0, 80.0, 90.0} - температурные стадии
thermreg:tempStages:2_50_80;
thermreg:setT:40; // -> thermreg setT {40.0} - одна температурная стадия
thermreg:changeMode:3; // -> thermreg changeMode {3} - меняет режим работы терморегулятора 
(0 - холостой ход;
 1 - изменение температуры исходя из набора этапов 
 2 - постепенное повышение температуры Tmin до Tmax с шагом dT;
 3 - постепенное снижение температуры с Tmax до Tmin с шагом dT;
 4 - чтения средней температуры со всех термопар
 5 - чтения температур со всех термопар)
thermreg:start; // -> thermreg start - запускает работу многостадийного терморегулятора (с блокировкой)
thermreg:stop; // -> thermreg stop - выключает многостадийного терморегулятор

MAX6675:start; // -> MAX6675 start - запуск чтения с термопар
MAX6675:stop;  // -> MAX6675 stop - прекращение чтения с термопар
MAX6675:readAvrTemp;  // -> MAX6675 readAvrTemp - переход в режим чтения средней температуры со всех термопар
MAX6675:readAllTemp;  // -> MAX6675 readAllTemp - переход в режим чтения температур со всех термопар

DAC:settings:0_3_1_20; // -> DAC settings {0.0, 5.0, 1.0, 20.0} - Vmin, Vmax, dV, holding time
DAC:setHoldingTime:10; // -> DAC setHoldingTime {10.0} - holding time
DAC:voltageSettings:4_1_2_2.5_3; // -> DAC voltageSettings {1.0, 2.0, 2.5, 3.0} - стадии по напряжению
DAC:setV:3; // -> DAC setV {1.0} - одна температурная стадия
DAC:changeMode:3; // -> DAC changeMode {3} - меняет режим работы ЦАП 
(0 - холостой ход;
 1 - изменение напряжения исходя из набора этапов 
 2 - постепенное повышение напряжения с Vmin до Vmax с шагом dV;
 3 - постепенное снижение напряжения с Vmax до Vmin с шагом dV;)
DAC:start; // -> DAC start - запускает работу многостадийного ЦАП (с блокировкой)
DAC:stop; // -> DAC stop - выключает многостадийного ЦАП

INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {20} {1.0} - количество циклов измерений (10) с периодом в 1.0 сек
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219 с определенным периодом
INA219:reading; // -> INA219 reading - считывает значения с INA219 с определенным периодом до прерывания
INA219:stop; // -> INA219 stop - выключает измерения датчика

MOSFET:changeMode:0; // -> MOSFET changeMode {0} - MOSFET-транзистор переключает все реле в другой режим измерений
(0 - режим измерения I(T);
 1 - режим измерения I(V))
*/