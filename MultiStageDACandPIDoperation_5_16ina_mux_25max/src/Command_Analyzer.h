#pragma once

#include <Arduino.h>
#include <Command_Parser_2.h>

namespace commandAnalyzer
{
    void analyzeCommand(ParsedCommand* result); //const commandParser &pars
    //void changeData(); //Device &d
}