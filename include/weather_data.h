#include <vector>
#include <string>
using namespace std;

#ifndef _WEATHER_DATA_H
#define _WEATHER_DATA_H

class Weather_data
{
    private:

    int  pressure, humidity, wind_deg, timezone_offset;
    float temp, wind_speed, lat, lon;
    vector<string> alert_events, alert_descriptions;
    vector<int> weather_ids; 
    
    public:
    Weather_data();

    void set_timezone_offset(int timezone_offset);
    void set_pressure(int pressure);
    void set_humidity(int humidity);
    void set_wind_deg(int wind_deg);
    void set_temp(float temp);
    void set_wind_speed(float wind_speed);
    void set_weather_id(vector<int> weather_ids);
    void set_alert_event(vector<string> alert_events);
    void set_alert_description(vector<string> alert_descriptions);
    void set_lat(float lat);
    void set_lon(float lon);

    int get_timezone_offset();
    int get_pressure();
    int get_humidity();
    int get_wind_deg();
    float get_temp();
    float get_wind_speed();
    float get_lat();
    float get_lon();
    vector<int> get_weather_id();
    vector<string> get_alert_event();
    vector<string> get_alert_description();
};
void list_vector(vector<int> vec);

#endif