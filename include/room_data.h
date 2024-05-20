#ifndef ROOM_DATA_H_
#define ROOM_DATA_H_

class Room_data
{
    private:

    float internal_temperature;
    float internal_humidity;
    float window_deg;
    int desired_temperature;
    bool is_auto;

    public:

    Room_data()
    {
        internal_temperature = 0;
        internal_humidity = 0;
        window_deg = 0;
        desired_temperature = 20;
        is_auto = true;
    }

    void set_internal_temperature(float internal_temperature);
    void set_internal_humidity(float internal_humidity);
    void set_window_deg(float window_deg);
    void set_desired_temperature(int desired_temperature);
    void set_is_auto(bool is_auto);

    float get_internal_temperature();
    float get_internal_humidity();
    float get_window_deg();
    int get_desired_temperature();
    bool get_is_auto();
};


#endif