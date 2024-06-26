
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include <SNTP.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "driver/gpio.h"

#include "WiFi_STA.h"
#include "HTTP_request_handler.h"
#include "room_data.h"
#include "weather_data.h"
#include "MQTT.h"
#include "HTTP_server.h"
#include "esp_https_server.h"
#include "credentials.h"
#include "store_data.h"
#include <bme680.h>
#include "bme680_sensor.h"
#include "motor_control.h"

#define LED_PIN GPIO_NUM_2

extern "C"
{
    void app_main(void);
    TaskHandle_t http_request_task_handle;
};

char def_ssid[33] = WIFI_SSID_DEFAULT;
char def_pass[64] = WIFI_PWD_DEFAULT;

Room_data Internal_room_data;
Weather_data Weather;

void setup_LED_GPIO()
{
    gpio_reset_pin(LED_PIN);
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_INPUT_OUTPUT);
    
}

void control_LED_task(void *params)
{
    while (1)
    {
        // Check if LED is turned on
        if (gpio_get_level(LED_PIN) == 1)
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level(LED_PIN, 0); // Turn LED off
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

//Setting up the flash memory.
void init_flash_settings()
{
    //Initializing the flash memory, and checks if the NVS partitions have been corrupted.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    Internal_room_data.set_window_deg(nvs_read_window_deg()/100);
    Internal_room_data.set_desired_temperature(nvs_read_desired_temp());
    Internal_room_data.set_is_auto(nvs_read_operation_mode());
    Weather.set_lat(nvs_read_latitude()/100);
    Weather.set_lon(nvs_read_longitude()/100);
}

//Starting the Wi-Fi module.
void init_wifi_settings(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    const char* ssid = nvs_read_wifi_ssid();
    const char* pass = nvs_read_wifi_pass();
    wifi_init_sta(ssid, pass);
}

void app_main(void)
{  
    //Initializing NVS
    init_flash_settings();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Loading the configuration page from the flash memory
    config_web_page_buffer();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Configuring the onboard LED
    setup_LED_GPIO();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Setting up the sensor
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(bme680_measure, "bme680_measure", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM); //2048?
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Initializing the Wi-Fi settings
    init_wifi_settings();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Creating the weather data request task
    xTaskCreate(&https_request_task, "https_request_task", 8192, NULL, 5, &http_request_task_handle);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Creating the LED turn-off task
    xTaskCreate(control_LED_task, "control_LED_task", 2048, NULL, 5, NULL);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //Getting the current time
    configure_time();
    xTaskCreate(motor_control_task, "motor_control_task", 4096, NULL, 3, NULL);
}