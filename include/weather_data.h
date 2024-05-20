#include <vector>
#include <string>
using namespace std;

#ifndef _WEATHER_DATA_H
#define _WEATHER_DATA_H

class Weather_data
{
    private:

    int  pressure, humidity, wind_deg, timezone_offset;
    float temp, wind_speed;
    vector<string> alert_events, alert_descriptions;
    vector<int> weather_ids; 
    
    public:
    Weather_data();

    void _set_timezone_offset(int timezone_offset);
    void _set_pressure(int pressure);
    void _set_humidity(int humidity);
    void _set_wind_deg(int wind_deg);
    void _set_temp(float temp);
    void _set_wind_speed(float wind_speed);
    void _set_weather_id(vector<int> weather_ids);
    void _set_alert_event(vector<string> alert_events);
    void _set_alert_description(vector<string> alert_descriptions);

    int _get_timezone_offset();
    int _get_pressure();
    int _get_humidity();
    int _get_wind_deg();
    float _get_temp();
    float _get_wind_speed();
    vector<int> _get_weather_id();
    vector<string> _get_alert_event();
    vector<string> _get_alert_description();
};
void _list_vector(vector<int> vec);

#endif