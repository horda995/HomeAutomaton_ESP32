/* This module is responsible for handling the Wi-Fi related parts of the project.
 * Source: https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_https_server.h>
#include "MQTT.h"
#include "credentials.h"

#define ESP_MAXIMUM_RETRY (10)
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;

static const char *STA_TAG = "WIFI ";
static int s_retry_num = 0;
const char ssid[32] = WIFI_SSID_DEFAULT;
const char pass[64] = WIFI_PWD_DEFAULT;

/*The function below handles the Wi-Fi related events, like connections, disconnections
 *timeouts, obtaining the ip. The initialization of the MQTT connection is also done here.
 */
static void wifi_sta_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(STA_TAG, "retry to connect to the AP");
        }
        else
        {
            ESP_LOGI(STA_TAG, "connect to the AP fail");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            s_retry_num = 0;
            ESP_ERROR_CHECK(esp_wifi_stop());
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(STA_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(STA_TAG, "got ip: starting MQTT Client\n");
        mqtt_app_start();
    }
}

//Initializing the device as a Wi-Fi station.
void wifi_init_sta(void)
{
    ESP_LOGI(STA_TAG, "ESP_WIFI_MODE_STA");
    s_wifi_event_group = xEventGroupCreate(); //This synchronizes Wi-Fi events.

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //Event handler registration.
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_sta_event_handler, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_sta_event_handler, &instance_got_ip));
    
    //Configuring the credentials and authentication mode.
    wifi_config_t wifi_config_sta = {};
    strcpy((char *)wifi_config_sta.sta.ssid, ssid);
    strcpy((char *)wifi_config_sta.sta.password, pass);
    wifi_config_sta.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config_sta.sta.pmf_cfg.capable = true;
    wifi_config_sta.sta.pmf_cfg.required = false;
    wifi_config_sta.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    //Starting the Wi-Fi in station mode.
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(STA_TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(STA_TAG, "connected to a:%s", ssid);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(STA_TAG, "Failed to connect to:%s", ssid);
    }
    else
    {
        ESP_LOGE(STA_TAG, "UNEXPECTED EVENT");
    }
}