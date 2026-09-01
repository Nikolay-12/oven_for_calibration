#include "Command_Analyzer.h"

namespace commandAnalyzer
{
    void analyzeCommand(ParsedCommand* cmd){
        if (strcmp(cmd->device_name, "thermreg") == 0) { 
            if (strcmp(cmd->main_key, "PIDsettings") == 0) {  //"thermreg:PIDsettings:5.0_0.01_0.04_0.5;"
                thermoRegulator::updatePIDcoef(cmd->float_data);
            } 
            else if (strcmp(cmd->main_key, "timeSettings") == 0) {  //"thermreg:timeSettings:10.0_20.0_2.0_100.0;"
                thermoRegulator::updateTimeSettings(cmd->float_data);
            }
            else if (strcmp(cmd->main_key, "tempSettings") == 0) { //"thermreg:tempSettings:5_40_50_60_80_90;"
                thermoRegulator::updateStages(cmd->left_data);
            }
            else if (strcmp(cmd->main_key, "tempSettingsAuto") == 0) { //"thermreg:tempSettingsAuto:20_40_100;"
                thermoRegulator::updateStagesAuto(cmd->float_data);
            }
            else if (strcmp(cmd->main_key, "setT") == 0) {  //"thermoreg:setT:40;"
                thermoRegulator::setTemp(cmd->float_data);
            }
            else if (strcmp(cmd->main_key, "start") == 0) {  //"thermoreg:start;"
                thermoRegulator::thermoregOn();
            }
            else if (strcmp(cmd->main_key, "stop") == 0) {  //"thermoreg:stop;"
                thermoRegulator::thermoregOff();
            }
        }
        //...        
    }
}

/*
Commands:
thermreg:PIDsettings:5.0_0.01_0.04_0.5; // new float Kp, Ki, Kd, dt 
thermreg:timeSettings:10.0_20.0_2.0_100.0; // new float stabilization_time, holding_time, temp_tolarance, max_stab_time
thermreg:tempSettings:5_40_50_60_80_90; // new int temperatures_count and float temperatures array
thermoreg:setT:40;
thermoreg:start;
thermoreg:stop;
*/