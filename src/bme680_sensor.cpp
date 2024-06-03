/* This module is responsible for measuring the gas resistance, which is directly
 * proportional with air quality. The project uses the Bosch BME680 sensor.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_log.h"
#include <bme680.h>
#include <string.h>
#include <room_data.h>

#define PORT (i2c_port_t)0
#define I2C_MASTER_SDA (gpio_num_t)21
#define I2C_MASTER_SCL (gpio_num_t)22
#define BME680_I2C_ADDR (i2c_port_t)0x77

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

extern Room_data Internal_room_data;

bme680_t sensor;
bme680_values_float_t values;
uint32_t duration;

void bme680_measure(void *pvParameters)
{
    bme680_t sensor;
    memset(&sensor, 0, sizeof(bme680_t));

    ESP_ERROR_CHECK(bme680_init_desc(&sensor, BME680_I2C_ADDR, PORT, I2C_MASTER_SDA, I2C_MASTER_SCL));

    //Initialize the sensor
    ESP_ERROR_CHECK_WITHOUT_ABORT(bme680_init_sensor(&sensor));

    //Changing the oversampling rates
    bme680_set_oversampling_rates(&sensor, BME680_OSR_4X, BME680_OSR_NONE, BME680_OSR_2X);

    //Configure the sensor
    bme680_set_filter_size(&sensor, BME680_IIR_SIZE_7);
    bme680_set_heater_profile(&sensor, 0, 200, 100);
    bme680_use_heater_profile(&sensor, 0);
    bme680_set_ambient_temperature(&sensor, 22);

    uint32_t duration;
    bme680_get_measurement_duration(&sensor, &duration);

    bme680_values_float_t values;
    while (1)
    {
        //Start the measurement cycle
        if (bme680_force_measurement(&sensor) == ESP_OK)
        {
            //Wait
            vTaskDelay(duration);

            //Store the results
            if (bme680_get_results_float(&sensor, &values) == ESP_OK)
            {
                ESP_LOGI("BME680", "BME680 Sensor: %.2f °C, %.2f %%, %.2f Ohm",
                values.temperature, values.humidity,  values.gas_resistance);
                Internal_room_data.set_internal_temperature(values.temperature);
                Internal_room_data.set_internal_humidity(values.humidity);
                Internal_room_data.set_gas_resistance(values.gas_resistance);
            }
            vTaskDelay(60000 / portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}
