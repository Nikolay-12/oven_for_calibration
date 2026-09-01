#include "Command_Analyzer.h"

namespace commandAnalyzer
{
    void analyzeCommand(ParsedCommand* cmd){
        // Command processing for thermoregulator
        if (strcmp(cmd->device_name, "thermreg") == 0) { 
            if (strcmp(cmd->main_key, "PIDsettings") == 0) {  //"thermreg:PIDsettings:5.0_0.01_0.04_0.5;"
                thermoRegulator::updatePIDcoef(cmd->float_data);
            } 
            else if (strcmp(cmd->main_key, "time&tolSettings") == 0) {  //"thermreg:time&tolSettings:10.0_20.0_2.0_100.0;"
                thermoRegulator::updateTimeSettings(cmd->float_data);
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
            else if (strcmp(cmd->main_key, "start") == 0) {  //"thermreg:start;"
                thermoRegulator::on();
            }
            else if (strcmp(cmd->main_key, "stop") == 0) {  //"thermreg:stop;"
                thermoRegulator::off();
            }

            else if (strcmp(cmd->main_key, "startTempReading") == 0) {  //"thermreg:startTempReading;"
                thermoRegulator::startTempReading();
            }
            else if (strcmp(cmd->main_key, "stopTempReading") == 0) {  //"thermreg:stopTempReading;"
                thermoRegulator::stopTempReading();
            }

            else if (strcmp(cmd->main_key, "readAvrTemp") == 0) {  //"thermreg:readAvrTemp;"
                thermoRegulator::readAllTempAndReturnAverage();
            }
            else if (strcmp(cmd->main_key, "readAllTemp") == 0) {  //"thermreg:readAllTemp;"
                thermoRegulator::readAllTempAndReturnThem();
            }
            else if (strcmp(cmd->main_key, "stopTempsReading") == 0) {  //"thermreg:stopTempsReading;"
                thermoRegulator::stopReadingAllTemps();
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
thermreg:PIDsettings:5.0_0.01_0.04_0.5; // new float Kp, Ki, Kd, dt 
thermreg:timeSettings:10.0_20.0_2.0_100.0; // new float stabilization_time, holding_time, temp_tolarance, max_stab_time
thermreg:tempSettings:5_40_50_60_80_90; // new int temperatures_count and float temperatures array
thermoreg:start;
thermoreg:stop;
INA219:read;
*/