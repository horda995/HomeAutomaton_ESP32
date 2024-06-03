/* This module is responsible for parsing the data requested from the Openweathermap API.
 * The server responds by sending the data in JSON format, which is then parsed using cJSON.
 */

#include "esp_log.h"
#include "cJSON.h"
#include "weather_data.h"
#include <string>
#include <string.h>
#include <vector>

using namespace std;

extern Weather_data Weather;

static const char *TAG = "JSON_PARSER";

void parse_weather_json(string response_string)
{   
    cJSON *current_JSON = NULL;
    cJSON *current_temp_JSON = NULL, *current_pressure = NULL, *current_humidity_JSON = NULL, *current_wind_speed_JSON = NULL, *current_wind_deg_JSON = NULL;
    cJSON *current_weathers_JSON = NULL, *current_weather = NULL, *current_weather_id_JSON = NULL;
    cJSON *alerts_JSON = NULL, *alert_JSON = NULL, *alert_event_JSON = NULL, *alert_description_JSON = NULL;

    char *response = (char*)malloc(4096+1);
    if (response == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory.");
        return;
    }

    
    size_t location = response_string.find("\r\n\r\n"); //Finding the location of the JSON data
    response_string = response_string.substr(location, response_string.length() - 1); 
    strcpy(response, response_string.c_str());
    cJSON *openweathermap_data = cJSON_Parse(response); //Parsing the JSON content from the HTTP response. 
    free(response);
    if (openweathermap_data == NULL)
    {
        ESP_LOGE(TAG, "%s",cJSON_GetErrorPtr() );
        ESP_LOGE(TAG, "Failed to get the JSON file from Openweathermap.");
        goto json_parse_end;
    }
   
    current_JSON = cJSON_GetObjectItem(openweathermap_data, "current");
    if (!cJSON_IsObject(current_JSON))
        ESP_LOGE(TAG, "Failed to get current weather data.");
    
    //Copying the data to an object member by member using a class Setter.
    current_temp_JSON = cJSON_GetObjectItem(current_JSON, "temp");
    if (cJSON_IsNumber(current_temp_JSON) )
    {
        ESP_LOGI(TAG, "Current temperature: %f", current_temp_JSON -> valuedouble);
        Weather.set_temp(current_temp_JSON -> valuedouble);
    }
    else
        ESP_LOGE(TAG, "Failed to get current_JSON temperature.");
    
    current_pressure = cJSON_GetObjectItem(current_JSON, "pressure");
    if (cJSON_IsNumber(current_pressure) )
    {
        ESP_LOGI(TAG, "Current pressure: %d", current_pressure -> valueint);
        Weather.set_pressure(current_pressure -> valueint);
    }
    else
        ESP_LOGE(TAG, "Failed to get current_JSON pressure.");
    
    current_humidity_JSON = cJSON_GetObjectItem(current_JSON, "humidity");
    if (cJSON_IsNumber(current_humidity_JSON) )
    {
        ESP_LOGI(TAG, "Current humidity: %d", current_humidity_JSON -> valueint);
        Weather.set_humidity(current_humidity_JSON -> valueint);
    }
    else
        ESP_LOGE(TAG, "Failed to get current_JSON humidity.");

    current_wind_speed_JSON = cJSON_GetObjectItem(current_JSON, "wind_speed");
    if (cJSON_IsNumber(current_wind_speed_JSON) )
    {
        ESP_LOGI(TAG, "Current wind speed: %f", current_wind_speed_JSON -> valuedouble);
        Weather.set_wind_speed(current_wind_speed_JSON -> valuedouble);
    }
    else
        ESP_LOGE(TAG, "Failed to get current_JSON wind speed.");

    current_wind_deg_JSON = cJSON_GetObjectItem(current_JSON, "wind_deg");
    if (cJSON_IsNumber(current_wind_deg_JSON) )
    {
        ESP_LOGI(TAG, "Current wind direction: %d", current_wind_deg_JSON -> valueint);
        Weather.set_wind_deg(current_wind_deg_JSON -> valueint);
    }
    else
        ESP_LOGE(TAG, "Failed to get current_JSON wind direction.");
    
    /*Because Openweathermap describes the weather by using multiple weather types if necessary, we store the weather IDs
     * and descriptions using vectors, which are dynamic arrays.
     */
    current_weathers_JSON = cJSON_GetObjectItem(current_JSON, "weather");
    if (!cJSON_IsArray(current_weathers_JSON) )
        ESP_LOGE(TAG, "Failed to get weather description.");
    else
    {
        vector<int> temp_id;
        ESP_LOGI(TAG, "Weather count: %d", cJSON_GetArraySize(current_weathers_JSON) );
        cJSON_ArrayForEach(current_weather, current_weathers_JSON)
        {
            current_weather_id_JSON = cJSON_GetObjectItem(current_weather, "id");
            if (cJSON_IsNumber(current_weather_id_JSON) )
            {
                ESP_LOGI(TAG, "Weather id: %d", current_weather_id_JSON -> valueint);
                temp_id.push_back(current_weather_id_JSON -> valueint);
            }
            else
                ESP_LOGE(TAG, "Failed to get weather id.");
        }
        Weather.set_weather_id(temp_id);
    }
    //We do similarly with weather alerts.
    alerts_JSON = cJSON_GetObjectItem(openweathermap_data, "alerts");
    if (alerts_JSON == NULL)
    {
        ESP_LOGI(TAG, "No weather alerts at the moment.");
    }
    else if (!cJSON_IsArray(alerts_JSON) )
        ESP_LOGE(TAG, "Failed to get weather alerts");
    else
    {
        vector<string> temp_event, temp_description;
        ESP_LOGI(TAG, "Alert count: %d", cJSON_GetArraySize(alerts_JSON) );
        cJSON_ArrayForEach(alert_JSON, alerts_JSON)
        {
            alert_event_JSON = cJSON_GetObjectItem(alert_JSON, "event");
            if (cJSON_IsString(alert_event_JSON) )
            {
                ESP_LOGI(TAG, "Alert event: %s", alert_event_JSON -> valuestring);
                temp_event.push_back(alert_event_JSON -> valuestring);
            }
            else
                ESP_LOGE(TAG, "Failed to get alert.");
            alert_description_JSON = cJSON_GetObjectItem(alert_JSON, "description");
            if (cJSON_IsString(alert_description_JSON) )
            {
                ESP_LOGI(TAG, "Alert description: %s", alert_description_JSON -> valuestring);
                temp_description.push_back(alert_description_JSON -> valuestring);
            }
            else
                ESP_LOGE(TAG, "Failed to get alert description.");
        }
        Weather.set_alert_event(temp_event);
        Weather.set_alert_description(temp_description);
    }
    json_parse_end:
    cJSON_Delete(openweathermap_data);
    
}