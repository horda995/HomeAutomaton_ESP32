/* This module is responsible for reading the temperature and humidity of a room using a DHT sensor. Used source:
 *
 * https://github.com/UncleRus/esp-idf-lib/blob/master/examples/dht/default/main/main.c
 * 
 */

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>
#include "esp_log.h"
#include "room_data.h"

#define TAG "DHT_TEMP_SENSOR"

//DHT sensors are able to get the temperature and humidity of the premises they are located in.
extern Room_data Internal_room_data;
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11; //Setting the DHT temperature sensor model(DHT11 or DHT22, the latter is more precise).
static const gpio_num_t dht_gpio = gpio_num_t(17); //Setting the GPIO pin used by the sensor.

void dht_measure(void *pvParameters)
{
    int16_t temperature = 0;
    int16_t humidity = 0;
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY); //It is necessary to set the GPIO pin to pullup-only mode.

    while (1)
    {
        /* Read from the sensor at a defined interval.
         * If the sensor is not available, send an error message.
         */
        if (dht_read_data(sensor_type, dht_gpio, &humidity, &temperature) == ESP_OK)
        {
            Internal_room_data.set_internal_temperature(temperature / 10);
            Internal_room_data.set_internal_humidity(humidity / 10);
            ESP_LOGI(TAG, "Humidity: %f%% Temp: %fC", Internal_room_data.get_internal_humidity(), Internal_room_data.get_internal_temperature());
            vTaskDelay(60000 / portTICK_PERIOD_MS);
        }
        else
        {
            ESP_LOGI(TAG, "Could not read data from sensor");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        } 
    }
}
