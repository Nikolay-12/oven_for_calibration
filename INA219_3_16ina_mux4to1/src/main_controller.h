#pragma once

#include "project_config.h"
#include "main_for_INA219.h"


namespace MainController 
{   
    enum Mode  {IDLE = 0, 
                IT_MODE = 1, // Режим измерения I(T)
                IV_MODE = 2}; // Режим измерения I(V)

    void init();

    void inProgress();
    void stop();

    void setState(bool new_state);
    bool getState();
    void setMainMode(const int new_mode);
    uint8_t getMainMode();

    // Проверка корректности команды включения
    bool isMode(Mode expectedMode);
};