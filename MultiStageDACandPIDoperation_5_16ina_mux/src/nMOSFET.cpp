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
        switch(state) {
            case 0:
                Serial.print("MOSFET-транзистор переключился в состояние "); Serial.println(state); //Serial.println(" (режим 1 -- измерение I(T))");
                break;
            case 1:
                Serial.print("MOSFET-транзистор переключился в состояние "); Serial.println(state); //Serial.println(" (режим 2 -- измерение I(V))"
                break;
        }
        digitalWrite(MOSFET_PIN, state);
    }
    bool getState() { return state; }
}
