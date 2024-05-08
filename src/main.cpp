
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include <esp_https_server.h>
#include <SNTP.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "WiFi_STA.h"
#include "HTTP_request_handler.h"
#include "DHT_sensor.h"
#include "room_data.h"
#include "MQTT.h"

#define BLINK_GPIO GPIO_NUM_2

extern "C"
{
    void app_main(void);
};

Room_data Internal_room_data;

//Setting up the flash memory.
void init_flash_settings()
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_INPUT_OUTPUT);
    //Initializing the flash memory, and checks if the NVS partitions have been corrupted.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
//Starting the Wi-Fi module.
void init_wifi_settings(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_sta();
}
void app_main(void)
{  
    init_flash_settings();
    init_wifi_settings();
    
    xTaskCreate(dht_measure, "dht_test", 8192, NULL, 5, NULL);
    xTaskCreate(&https_request_task, "https_get_task", 8192, NULL, 5, NULL);
    xTaskCreate(&check_LED_task, "check_LED_task", 4096, NULL, 5, NULL);
    configure_time();   
}