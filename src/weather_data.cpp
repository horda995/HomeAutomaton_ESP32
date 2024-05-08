//The data that is acquired from the Openweathermap API is stored in a class instance.
#include <vector>
#include <string>
#include "weather_data.h"
#include "esp_log.h"
using namespace std;

static const char* TAG = "VECTOR LISTING:";

void Weather_data::_set_timezone_offset(int timezone_offset)
{
    this -> timezone_offset = timezone_offset;
}
void Weather_data::_set_pressure(int pressure)
{
    this -> pressure = pressure;
}
void Weather_data::_set_humidity(int humidity)
{
    this -> humidity = humidity;
}
void Weather_data::_set_wind_deg(int wind_deg)
{
    this -> wind_deg = wind_deg;
}
void Weather_data::_set_temp(float temp)
{
    this -> temp = temp;
}
void Weather_data::_set_wind_speed(float wind_speed)
{
    this -> wind_speed = wind_speed;
}
void Weather_data::_set_weather_id(vector<int> weather_ids)
{
    this -> weather_ids = weather_ids;
}
void Weather_data::_set_alert_event(vector<string> alert_events)
{
    this -> alert_events = alert_events;
}
void Weather_data::_set_alert_description(vector<string> alert_descriptions)
{
    this -> alert_descriptions = alert_descriptions;
}
int Weather_data::_get_timezone_offset()
{
    return timezone_offset;
}
int Weather_data::_get_pressure()
{
    return pressure;
}
int Weather_data::_get_humidity()
{
    return humidity;
}
int Weather_data::_get_wind_deg()
{
    return wind_deg;
}
float Weather_data::_get_temp()
{
    return temp;
}
float Weather_data::_get_wind_speed()
{
    return wind_speed;
}
vector<int> Weather_data::_get_weather_id()
{
    return weather_ids;
}
vector<string> Weather_data::_get_alert_event()
{
    return alert_events;
}
vector<string> Weather_data::_get_alert_description()
{
    return alert_descriptions;
}

Weather_data::Weather_data()
{
    pressure = humidity = wind_deg = timezone_offset = 0;
    temp = wind_speed = 0;
}

Weather_data Weather;

void _list_vector(vector<int> vec)
{
    for(int i = 0; i < vec.size(); i++)
    {
        ESP_LOGI(TAG, "%d", vec[i] );
    }
}