#include "Command_Analyzer.h"
#include "main_controller.h"
#include "main_for_INA219.h"
#include "System_State.h"

namespace commandAnalyzer
{
    bool cmdBlockForITmode = false;
    bool cmdBlockForIVmode = false;

    void analyzeCommand(ParsedCommand* cmd) {
        // Command processing for main controller
        if (strcmp(cmd->device_name, "mainController") == 0) { 
            if (strcmp(cmd->main_key, "setMainMode") == 0) {  //"mainController:setMainMode:1;"
                MainController::setMainMode(cmd->int_data[0]);
            }
            else if (strcmp(cmd->main_key, "stop") == 0) {  //"mainController:stop;"
                MainController::stop();
            }
        }


        // Command processing for INA219
        if (strcmp(cmd->device_name, "INA219") == 0) { 
            // Команда stop всегда разрешена
            if (strcmp(cmd->main_key, "stop") == 0) { 
                INA219_dop::off();
                SystemState::free();
                return;
            }
            
            // Команда status разрешена даже во время измерений
            if (strcmp(cmd->main_key, "status") == 0) {
                if (SystemState::canExecuteCommand("INA219", "status", SystemState::CommandType::ALLOWED_WHEN_BUSY)) {
                    // INA219_dop::printStatus();
                }
                return;
            }
            
            // Остальные команды блокируются во время измерений
            if (!SystemState::canExecuteCommand("INA219", cmd->main_key, SystemState::CommandType::BLOCKED_WHEN_BUSY)) {
                return;
            }

            if (strcmp(cmd->main_key, "readNtimes") == 0) { // команда "INA219:readNtimes;" занимает систему для измерений датчиком INA219
                INA219_dop::startReadingSeveralTimes();
                SystemState::occupy("INA219");

            }
            else if (strcmp(cmd->main_key, "reading") == 0) { // команда "INA219:reading;" занимает систему для измерений датчиком INA219
                INA219_dop::startEndlessReading();
                SystemState::occupy("INA219");
            }

            else if (strcmp(cmd->main_key, "measurementSettings") == 0) { // "INA219:measurementSettings:10_1.0;"
                INA219_dop::updateMeasurementSettings(cmd->int_data[0], cmd->float_data[0]);
            }
        }
    }
}

/*
Commands:
INA219:measurementSettings:20_1.0; // -> DAC measurementSettings {10, 1.0} - количество циклов измерений и перерыв между ними
INA219:readNtimes; // -> INA219 readNtimes - считывает N раз значения с INA219 с опредленным периодом
INA219:reading; // -> INA219 reading - считывает значения с INA219 с опредленным периодом до прерывания
INA219:stop; // -> INA219 stop - прерывание считывания
*/