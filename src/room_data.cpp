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
void Room_data::set_window_deg(float window_deg)
{
    this -> window_deg = window_deg;
}

float Room_data::get_internal_temperature()
{
    return internal_temperature;
}
float Room_data::get_internal_humidity()
{
    return internal_humidity;
}
float Room_data::get_window_deg()
{
    return window_deg;
}