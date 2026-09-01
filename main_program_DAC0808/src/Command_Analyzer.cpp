#include "Command_Analyzer.h"
#include "main_controller.h"
#include "Thermodat.h"
#include "main_for_DAC0808.h"
#include "main_for_INA219.h"
#include "nMOSFET.h"
#include "System_State.h"

namespace DAC = DAC0808;

namespace commandAnalyzer
{
    bool cmdBlockForITmode = false;
    bool cmdBlockForIVmode = false;

    void analyzeCommand(ParsedCommand* cmd) {
        // Command processing for main controller
        if (strcmp(cmd->device_name, "mainController") == 0) { 
            // Команда, которая всегда разрешена
            if (strcmp(cmd->main_key, "stop") == 0) { // команда "mainController:stop;" останавливает любые процессы
                MainController::stop();
                return;
            }

            // Пример записи команды устройству, которая будет исполняться даже во время измерений
            if (strcmp(cmd->main_key, "status") == 0) {
                if (SystemState::canExecuteCommand("mainController", "status", SystemState::CommandType::ALLOWED_WHEN_BUSY)) {
                    // MainController::printStatus();
                }
                return;
            }

            // Остальные команды блокируются во время измерений
            if (!SystemState::canExecuteCommand("mainController", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) { // проверка доступа к главному обработчику для смены режима
                return;
            }
            
            if (strcmp(cmd->main_key, "setMainMode") == 0) {  //"mainController:setMainMode:1;"
                MainController::setMainMode(cmd->int_data[0]);
            }
        }
        
        // Command processing for thermoregulator
        if (strcmp(cmd->device_name, "thermreg") == 0) { 
            if (strcmp(cmd->main_key, "stop") == 0) { // команда "thermoreg:stop;" предоставляет возможность начать другие измерения
                thermoRegulator::off();
                //SystemState::free();
                return;
            }
            
            if (!SystemState::canExecuteCommand("thermreg", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) { // проверка доступа к терморегулятору
                return;
            }

            if (strcmp(cmd->main_key, "start") == 0) { // команда "thermoreg:start;" занимает систему для измерений терморегулятором
                if (MainController::canStartThermoreg()) { // Проверка на возможность запуска
                    thermoRegulator::on();
                    SystemState::occupy("thermreg");
                }
            }

            if (strcmp(cmd->main_key, "PIDsettings") == 0) {  //"thermreg:PIDsettings:5.0_0.01_0.04_0.5;"
                thermoRegulator::updatePIDcoef(cmd->float_data);
            } 
            else if (strcmp(cmd->main_key, "time&tolSettings") == 0) {  //"thermreg:time&tolSettings:10.0_20.0_2.0_100.0;"
                thermoRegulator::updateTimeTolSettings(cmd->float_data);
            }
            else if (strcmp(cmd->main_key, "tempSettings") == 0) { //"thermreg:tempSettings:20_80_10;"
                thermoRegulator::updateTempSettings(cmd->float_data);
            }
            else if (strcmp(cmd->main_key, "tempStages") == 0) { // "thermreg:tempStages:5_40_50_60_80_90;" 
                thermoRegulator::updateStages(cmd->left_data);
            }
            else if (strcmp(cmd->main_key, "setT") == 0) {  //"thermreg:setT:40;"
                thermoRegulator::setTempPoint(cmd->float_data[0]);
            }
            else if (strcmp(cmd->main_key, "changeMode") == 0) {  //"thermreg:changeMode:3;"
                thermoRegulator::changeMode(cmd->int_data[0]);
            }   
        }

        // Command processing for MAX6675
        if (strcmp(cmd->device_name, "MAX6675") == 0) { 
            if (strcmp(cmd->main_key, "stop") == 0) { // команда "MAX6675:stop;" предоставляет возможность начать другие измерения
                thermoRegulator::stopTempReading();
                //SystemState::free();
                return;
            }
            
            if (!SystemState::canExecuteCommand("MAX6675", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) { // проверка доступа к термопарам
                return;
            }

            else if (strcmp(cmd->main_key, "start") == 0) {  // команда "MAX6675:start;" занимает систему для измерений термопарами
                if (MainController::canStartThermoreg()) { // Проверка на возможность запуска
                    thermoRegulator::startTempReading();
                    SystemState::occupy("MAX6675");
                }  
            }
            else if (strcmp(cmd->main_key, "readAvrTemp") == 0) {  // команда "MAX6675:readAvrTemp;" занимает систему для измерений средней температуры со всех термопар
                thermoRegulator::readAvrTemp();
            }
            else if (strcmp(cmd->main_key, "readAllTemp") == 0) { // команда "MAX6675:readAllTemp;" занимает систему для измерений всех температур
                thermoRegulator::readAllTemps();
            }
        }

        // Command processing for DAC
        if (strcmp(cmd->device_name, "DAC") == 0) { 
            if (strcmp(cmd->main_key, "stop") == 0) { // команда "DAC:stop;" предоставляет возможность начать другие измерения
                DAC::off();
                //SystemState::free();
                return;
            }
            
            if (!SystemState::canExecuteCommand("DAC", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) {
                return;
            }

            if (strcmp(cmd->main_key, "start") == 0) { // команда "DAC:start;" занимает систему для измерений ЦАПом
                if (MainController::canStartThermoreg()) { // Проверка на возможность запуска
                    DAC::on();
                    SystemState::occupy("DAC");
                }
            }

            if (strcmp(cmd->main_key, "settings") == 0) {  //"DAC:settings:0_3_1_20;"
                DAC::updateSettings(cmd->float_data);
            } 
            else if (strcmp(cmd->main_key, "setHoldingTime") == 0) {  //"DAC:setHoldingTime:10;"
                DAC::updateTimeSettings(cmd->float_data[0]);
            } 
            else if (strcmp(cmd->main_key, "voltageSettings") == 0) { //"DAC:voltageSettings:5_1_2_2.5_3_5;"
                DAC::updateStages(cmd->left_data);
            }
            else if (strcmp(cmd->main_key, "setV") == 0) {  //"DAC:setV:3;"
                DAC::setVoltagePoint(cmd->float_data[0]);
            }
            else if (strcmp(cmd->main_key, "changeMode") == 0) {  //"DAC:changeMode:3;"
                DAC::changeMode(cmd->int_data[0]);
            }
        }

        // Command processing for INA219
        if (strcmp(cmd->device_name, "INA219") == 0) {   
            if (strcmp(cmd->main_key, "stop") == 0) { // команда "INA219:stop;" предоставляет возможность начать другие измерения
                INA219controller::off();
                //SystemState::free();
                return;
            }
            
            if (!SystemState::canExecuteCommand("INA219", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) {
                return;
            }

            if (strcmp(cmd->main_key, "readNtimes") == 0) { // команда "INA219:readNtimes;" занимает систему для измерений датчиком INA219
                INA219controller::startReadingSeveralTimes();
                SystemState::occupy("INA219");

            }
            else if (strcmp(cmd->main_key, "reading") == 0) { // команда "INA219:reading;" занимает систему для измерений датчиком INA219
                INA219controller::startEndlessReading();
                SystemState::occupy("INA219");
            }

            else if (strcmp(cmd->main_key, "measurementSettings") == 0) { // "INA219:measurementSettings:10_1.0;"
                INA219controller::updateMeasurementSettings(cmd->int_data[0], cmd->float_data[0]);
            }
        }

        // Command processing for MOSFET
        if (strcmp(cmd->device_name, "MOSFET") == 0) { 
            if (!SystemState::canExecuteCommand("MOSFET", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) {
                return;
            }

            if (strcmp(cmd->main_key, "changeMode") == 0) { // "MOSFET:changeMode:0;"
                MOSFET::changeState(cmd->bool_data[0]);
            }
        }
    }
}

/*
Commands:
Управление главным обработчиком:
mainController:stop; // -> mainController stop
mainController:setMainMode:1; // -> mainController setMainMode {1}

Управление терморегулятором:
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

Управление термопарами (отдельно):
MAX6675:start;
MAX6675:stop;
MAX6675:readAvrTemp;
MAX6675:readAllTemp;

Управление ЦАПом:
DAC:settings:0_3_1_20; // -> DAC settings {0.0, 5.0, 1.0, 20.0} - Vmin, Vmax, dV, holding time
DAC:setHoldingTime:10; // -> DAC setHoldingTime {10.0}
DAC:voltageSettings:5_1_2_2.5_3_5; // -> DAC settings {1.0, 2.0, 2.5, 3.0, 5.0}
DAC:setV:3; // -> DAC setV {1.0}
DAC:changeMode:3; // -> DAC changeMode {3} - меняет режим работы ЦАП 
(1 - устанавливает напряжение на выходе ЦАП на уровне V;
 2 - постепенное повышение напряжения с Vmin до Vmax с шагом dV;
 3 - постепенное снижение напряжения с Vmax до Vmin с шагом dV;
 4 - изменение напряжения исходя из набора этапов)
DAC:start; // -> DAC start
DAC:stop; // -> DAC stop

Управление датчиками тока (отдельно):
INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {10, 1.0} - количество циклов измерений и перерыв между ними
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219 с опредленным периодом
INA219:reading; // -> INA219 reading - считывает значения с INA219 с опредленным периодом до прерывания
INA219:stop; // -> INA219 stop - прерывание считывания

Управление MOSFET-транзистором:
MOSFET:changeMode:0;
*/