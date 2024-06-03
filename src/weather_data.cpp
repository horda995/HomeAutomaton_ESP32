//The data that is acquired from the Openweathermap API is stored in a class instance.
#include <vector>
#include <string>
#include "weather_data.h"
#include "esp_log.h"
#include "credentials.h"
using namespace std;

static const char* TAG = "VECTOR LISTING:";

void Weather_data::set_timezone_offset(int timezone_offset)
{
    this -> timezone_offset = timezone_offset;
}
void Weather_data::set_pressure(int pressure)
{
    this -> pressure = pressure;
}
void Weather_data::set_humidity(int humidity)
{
    this -> humidity = humidity;
}
void Weather_data::set_wind_deg(int wind_deg)
{
    this -> wind_deg = wind_deg;
}
void Weather_data::set_temp(float temp)
{
    this -> temp = temp;
}
void Weather_data::set_wind_speed(float wind_speed)
{
    this -> wind_speed = wind_speed;
}
void Weather_data::set_weather_id(vector<int> weather_ids)
{
    this -> weather_ids = weather_ids;
}
void Weather_data::set_alert_event(vector<string> alert_events)
{
    this -> alert_events = alert_events;
}
void Weather_data::set_alert_description(vector<string> alert_descriptions)
{
    this -> alert_descriptions = alert_descriptions;
}
void Weather_data::set_lat(float lat)
{
    this -> lat = lat;
}
void Weather_data::set_lon(float lon)
{
    this -> lon = lon;
}

int Weather_data::get_timezone_offset()
{
    return timezone_offset;
}
int Weather_data::get_pressure()
{
    return pressure;
}
int Weather_data::get_humidity()
{
    return humidity;
}
int Weather_data::get_wind_deg()
{
    return wind_deg;
}
float Weather_data::get_temp()
{
    return temp;
}
float Weather_data::get_wind_speed()
{
    return wind_speed;
}
vector<int> Weather_data::get_weather_id()
{
    return weather_ids;
}
vector<string> Weather_data::get_alert_event()
{
    return alert_events;
}
vector<string> Weather_data::get_alert_description()
{
    return alert_descriptions;
}
float Weather_data::get_lat()
{
    return lat;
}
float Weather_data::get_lon()
{
    return lon;
}

Weather_data::Weather_data()
{
    pressure = humidity = wind_deg = timezone_offset = 0;
    temp = wind_speed = 0;
    lat = LAT;
    lon = LON;
}

void list_vector(vector<int> vec)
{
    for(int i = 0; i < vec.size(); i++)
    {
        ESP_LOGI(TAG, "%d", vec[i] );
    }
}