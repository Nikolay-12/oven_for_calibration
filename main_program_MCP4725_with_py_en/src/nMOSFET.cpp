#include "nMOSFET.h"
#include "main_controller.h"


namespace MOSFET
{
    bool state;
    
    void init() {
        pinMode(MOSFET_PIN, OUTPUT);
        digitalWrite(MOSFET_PIN, state);
    }

    void changeState(bool new_state) {
        state = new_state;
        Serial.print("The MOSFET transistor has switched to state "); Serial.println(state);
        digitalWrite(MOSFET_PIN, state);
    }
    bool getState() { return state; }
}
