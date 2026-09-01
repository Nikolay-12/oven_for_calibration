#pragma once
#include <Arduino.h>
#include "project_config.h"

namespace MuxController {
    void init();
    
    void selectChannel(uint8_t channel); // 0-3
    
    uint8_t getCurrentChannel();
}
