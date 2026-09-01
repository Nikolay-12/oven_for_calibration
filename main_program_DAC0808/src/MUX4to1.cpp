#include "MUX4to1.h"

namespace MuxController {
    uint8_t currentChannel = 0;
    
    void init() {
        pinMode(ESP_MUX8to2_S0, OUTPUT);
        pinMode(ESP_MUX8to2_S1, OUTPUT);
        
        
        selectChannel(currentChannel); // По умолчанию канал 0
    }
    
    void selectChannel(uint8_t channel) {
        if (channel >= MUX_CHANNELS) return;
        
        currentChannel = channel;
        digitalWrite(ESP_MUX8to2_S0, (channel & 0x01) ? HIGH : LOW);
        digitalWrite(ESP_MUX8to2_S1, (channel & 0x02) ? HIGH : LOW);
        delayMicroseconds(50); // Небольшая задержка для переключения
    }
    
    uint8_t getCurrentChannel() {
        return currentChannel;
    }
}