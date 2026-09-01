#pragma once
#include <stdint.h>
#include "thermocouples.h"


namespace thermocouple_MAX6675
{
  void init();
  double readCelsius();
}
