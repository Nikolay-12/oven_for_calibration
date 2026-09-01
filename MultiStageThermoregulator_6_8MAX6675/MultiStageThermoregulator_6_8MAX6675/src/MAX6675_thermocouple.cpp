#include "MAX6675_thermocouple.h"
#include <SPI.h>
//#include <Arduino.h>


namespace thermocouple_MAX6675
{
  void init()
  {
    pinMode(CLK_PIN, OUTPUT);
    digitalWrite(CLK_PIN, HIGH);
  }

  double readCelsius() //220-230 ms
  {
    uint16_t v;
    
    v = SPI.transfer(0x00);
    v <<= 8;
    v |= SPI.transfer(0x00);
    
    if (v & 0x4) {
        // uh oh, no thermocouple attached!
        return NAN; 
    }

    v >>= 3;
    return v*0.25;
  }
}
