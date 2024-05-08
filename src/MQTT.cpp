/* This module is responsible for handling everything that is MQTT related. The project
 * uses the MQTT protocol as a means of communication between the phone application and the microcontroller.
 * It is a lightweight protocol useful where it is necessary to be use as few resources as possible.
 * Source: https://github.com/espressif/esp-idf/tree/release/v5.1/examples/protocols/mqtt
 */
#include <stdio.h>
#include <stdint.h>
#include "cJSON.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <string>
#include "driver/gpio.h"
#include "room_data.h"
#include "weather_data.h"
#include "credentials.h"


#define BLINK_GPIO GPIO_NUM_2
static const char *TAG = "MQTT_HANDLER";
static const char *topic_address = MQTT_TOPIC_ADDRESS;
static const char *topic = MQTT_TOPIC;
static const char *address_uri = MQTT_ADDRESS_URI;
uint32_t MQTT_CONNECTED = 0;
string JSON_data = " ";
float window_deg = 0;
extern Room_data Internal_room_data;
extern Weather_data Weather;
esp_mqtt_client_handle_t client = NULL;
void Publisher_Task(void *params);

//Parsing the window position data from the acquired JSON file.
void mqtt_json_parser(const char* const json_data){
    const cJSON *window_deg = NULL, *phone_data = NULL;
    cJSON *data = cJSON_Parse(json_data);
    if (data == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL){
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
        goto end;
    }
    phone_data = cJSON_GetObjectItemCaseSensitive(data, topic);
    if (phone_data == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL){
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
        goto end;
    }
    else
        window_deg = cJSON_GetObjectItemCaseSensitive(phone_data, "windowDeg");
    
    //Writing the parsed data to a class member, and making the onboard LED blink once.
    if (cJSON_IsNumber(window_deg)){
        ESP_LOGI("MQTT_JSON_PARSER:", "window_deg: %f", window_deg->valuedouble);
        if (window_deg->valuedouble != Internal_room_data.get_window_deg())
        {
            Internal_room_data.set_window_deg(window_deg->valuedouble);
            gpio_set_level(BLINK_GPIO, 1);
        }
    }
    end:
    cJSON_Delete(data);
}
void check_LED_task(void *Params)
{
    //Turning the onboard LED off, if it was turned on before.
    while(1)
    {
        if (gpio_get_level(BLINK_GPIO) == 1)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(BLINK_GPIO, 0);
        }
    }
}

void MQTT_publish()
{
    if (MQTT_CONNECTED)
    {
        /*Because the weather and weather alert related members of the class object are dynamic arrays,
            *it is necessary to treat them as such.
            */
        string weather_id_JSON =  "[", weather_alert_event_JSON = "[";
        for (int i =0; i < Weather._get_weather_id().capacity(); i++)
        {
            weather_id_JSON += to_string(Weather._get_weather_id()[i]);
            if (i < Weather._get_weather_id().capacity() - 1)
                weather_id_JSON += ",";
        }
        for (int i =0; i < Weather._get_alert_event().capacity(); i++)
        {
            weather_alert_event_JSON += "\"" + Weather._get_alert_event()[i] + "\"" ;
            if (i < Weather._get_alert_event().capacity() - 1)
                weather_alert_event_JSON += ",";
        }       
        weather_id_JSON += "]";
        weather_alert_event_JSON += "]";

        string internal_data_JSON = "{\"internal_temperature\":" + to_string(Internal_room_data.get_internal_temperature()) + ",\"internal_humidity\":" + to_string(Internal_room_data.get_internal_humidity()) + "}";
        string weather_data_JSON = "{\"weather_temperature\":" + to_string(Weather._get_temp()) + ",\"weather_humidity\":" + to_string(Weather._get_humidity()) +
                                    ",\"weather_wind_speed\":"+ to_string(Weather._get_wind_speed()) + ",\"weather_wind_deg\":" + to_string(Weather._get_wind_deg()) + 
                                    ",\"weather_id\":" + weather_id_JSON + ",\"weather_alert_event\":" + weather_alert_event_JSON +"}";
        string MCU_data_JSON = "{\"internal_data\":" + internal_data_JSON + ",\"weather_data\":" + weather_data_JSON + "}" ;

        esp_mqtt_client_publish(client, "/topic/MCU_data", MCU_data_JSON.c_str(), 0, 0, 0);
    }
}

//Responsible for the handling of the MQTT connection.
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            MQTT_CONNECTED=1;
            msg_id = esp_mqtt_client_subscribe(client, topic_address, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            TaskHandle_t Publisher_Task_Handle;
            xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, &Publisher_Task_Handle);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            vTaskDelete(Publisher_Task_Handle);
            MQTT_CONNECTED=0;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            mqtt_json_parser(event->data);
            MQTT_publish();
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqttConfig = {};
    mqttConfig.broker.address.uri = address_uri;
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
/* The following function is responsible for constructing the data formatted in JSON,
 * then publishing it periodically to the MQTT broker.
 */


void Publisher_Task(void *params)
{
    while (true)
    {
        MQTT_publish();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}