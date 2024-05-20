#ifndef STORE_DATA_H_
#define STORE_DATA_H_

void nvs_write_wifi_ssid(const char* ssid);
void nvs_write_wifi_pass(const char* pass);
void nvs_write_apikey(const char* apikey);
void nvs_write_operation_mode(bool op_mode);
void nvs_write_window_deg(float window_deg);
void nvs_write_desired_temp(int desired_temp);
void nvs_write_latitude(float lat);
void nvs_write_longitude(float lon);
void nvs_write_timezone(const char* tz);

const char* nvs_read_wifi_ssid();
const char* nvs_read_wifi_pass();
const char* nvs_read_apikey();
bool nvs_read_operation_mode();
float nvs_read_window_deg();
int nvs_read_desired_temp();
float nvs_read_latitude();
float nvs_read_longitude();
const char* nvs_read_timezone();

#endif