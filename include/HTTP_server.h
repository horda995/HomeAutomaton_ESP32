#include <esp_event.h>
#include <esp_https_server.h>

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

static httpd_handle_t server = NULL;

void config_web_page_buffer();
void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
httpd_handle_t start_webserver(void);

#endif