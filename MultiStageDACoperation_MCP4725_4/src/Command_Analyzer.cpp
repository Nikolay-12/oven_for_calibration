#include "Command_Analyzer.h"
#include "main_for_DAC_MCP4725.h"
#include "main_for_INA219.h"

namespace commandAnalyzer
{
    void analyzeCommand(ParsedCommand* cmd){
        // Command processing for DAC_MCP4725
        if (strcmp(cmd->device_name, "DAC") == 0) { 
            if (strcmp(cmd->main_key, "settings") == 0) {  //"DAC:settings:0_3_1_20;"
                DAC_MCP4725::updateSettings(cmd->float_data);
            } 
            else if (strcmp(cmd->main_key, "setHoldingTime") == 0) {  //"DAC:setHoldingTime:10;"
                DAC_MCP4725::updateTimeSettings(cmd->float_data[0]);
            } 
            else if (strcmp(cmd->main_key, "voltageSettings") == 0) { //"DAC:voltageSettings:5_1_2_2.5_3_5;"
                DAC_MCP4725::updateStages(cmd->left_data);
            }
            else if (strcmp(cmd->main_key, "setV") == 0) {  //"DAC:setV:3;"
                DAC_MCP4725::setVoltagePoint(cmd->float_data[0]);
            }
            else if (strcmp(cmd->main_key, "changeMode") == 0) {  //"DAC:changeMode:3;"
                DAC_MCP4725::changeMode(cmd->int_data[0]);
            }
            else if (strcmp(cmd->main_key, "start") == 0) {  //"DAC:start;"
                DAC_MCP4725::on();
            }
            else if (strcmp(cmd->main_key, "stop") == 0) {  //"DAC:stop;"
                DAC_MCP4725::off();
            }
        }
        //...   

        // Command processing for INA219
        if (strcmp(cmd->device_name, "INA219") == 0) {
            if (strcmp(cmd->main_key, "measurementSettings") == 0) { //"INA219:measurementSettings:10_1.0;"
                INA219_dop::updateMeasurementSettings(cmd->int_data[0], cmd->float_data[0]);
            }
            else if (strcmp(cmd->main_key, "readNtimes") == 0) {
                INA219_dop::startReadingSeveralTimes(); //"INA219:readNtimes;"
            } 
            else if (strcmp(cmd->main_key, "reading") == 0) {
                INA219_dop::startEndlessReading(); //"INA219:reading;"
            }
            else if (strcmp(cmd->main_key, "stop") == 0) {  //"INA219:stop;"
                INA219_dop::INA219off();
            }
        }     
    }
}

/*
Commands:
DAC:settings:0_3_1_20; // -> DAC settings {0.0, 5.0, 1.0, 20.0} - Vmin, Vmax, dV, holding time
DAC:measurementSettings:10_1.0; // -> DAC measurementSettings {10} - number of measurement cycles
DAC:voltageSettings:5_1_2_2.5_3_5; // -> DAC settings {1.0, 2.0, 2.5, 3.0, 5.0}
DAC:voltageSettingAuto:5_0_5; // -> нужно ли?
DAC:setV:3; // -> DAC setV {1.0}
DAC:changeMode:3; // -> DAC changeMode {3} - меняет режим работы ЦАП 
(1 - устанавливает напряжение на выходе ЦАП на уровне V;
 2 - постепенное повышение напряжения с Vmin до Vmax с шагом dV;
 3 - постепенное снижение напряжения с Vmax до Vmin с шагом dV;
 4 - изменение напряжения исходя из набора этапов)
DAC:start; // -> DAC start
DAC:stop; // -> DAC stop

INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219
INA219:reading; // -> INA219 reading  - считывает значения с INA219 до прерывания
INA219:stop; // -> INA219 stop - прерывание считывания
*/