#ifndef WIFI_STA_H_
#define WIFI_STA_H_

void wifi_init_sta(const char* ssid, const char* pass);
void wifi_restart(const char *ssid, const char *pass);

#endif