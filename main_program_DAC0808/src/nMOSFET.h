#pragma once

#include "project_config.h"

namespace MOSFET
{
    void init();
    void changeState(bool state);
    bool getState();
};