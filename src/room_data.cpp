//The data of the premises that the microcontroller and sensor are located in are stored in a class instance.
#include "room_data.h"

void Room_data::set_internal_temperature(float internal_temperature)
{
    this -> internal_temperature = internal_temperature;
}

void Room_data::set_internal_humidity(float internal_humidity)
{
    this -> internal_humidity = internal_humidity;
}

void Room_data::set_gas_resistance(float gas_resistance)
{
    this -> gas_resistance = gas_resistance;
}

void Room_data::set_window_deg(float window_deg)
{
    this -> window_deg = window_deg;
}

void Room_data::set_desired_temperature(int desired_temperature)
{
    this -> desired_temperature = desired_temperature;
}

void Room_data::set_is_auto(bool is_auto)
{
    this -> is_auto = is_auto;
}


float Room_data::get_internal_temperature()
{
    return internal_temperature;
}

float Room_data::get_internal_humidity()
{
    return internal_humidity;
}

float Room_data::get_gas_resistance()
{
    return gas_resistance;
}

float Room_data::get_window_deg()
{
    return window_deg;
}

int Room_data::get_desired_temperature()
{
    return desired_temperature;
}

bool Room_data::get_is_auto()
{
    return is_auto;
}