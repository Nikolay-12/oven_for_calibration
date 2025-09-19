#pragma once

#define THERMOCOUPLES_NUM 8

namespace thermocouples
{
    void init();
    void read_all_temp_once();
    void read_all_temp();
    void read_temp_from_one_thermocouple_once(thermocouple_index);
    void read_temp_from_one_thermocouple(thermocouple_index);
    void stop_reading_temp();
    float get_temp_celsius(thermocouple_index);
}

