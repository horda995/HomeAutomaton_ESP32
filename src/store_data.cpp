/* This module makes the handling of the NVS storage possible so that the 
 * microcontroller can store data crucial to its operation.
 */
#include "esp_log.h"
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string>


using namespace std;

#define TAG "NVS"

//Initializing the storage
esp_err_t init_nvs()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    return ret;
}

//Writing the ssid to the storage
void nvs_write_wifi_ssid(const char* ssid)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the Wi-Fi SSID in the NVS");
        ret = nvs_set_str(nvs_handle, "ssid", ssid);
        ESP_LOGI(TAG, "Updated ssid: %s", ssid);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the wifi password to the storage
void nvs_write_wifi_pass(const char* pass)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the Wi-Fi password in the NVS: ***");
        ret = nvs_set_str(nvs_handle, "pass", pass);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the Openweathermap API-key to the storage
void nvs_write_apikey(const char* apikey)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the Openweathermap API-key in the NVS");
        ret = nvs_set_str(nvs_handle, "apikey", apikey);
        ESP_LOGI(TAG, "Updated apikey: %s", apikey);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the operation mode to the storage
void nvs_write_operation_mode(bool op_mode)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the operation mode in the NVS.");
        if (op_mode)
        {
            ret = nvs_set_i8(nvs_handle, "op_mode", 1);
            ESP_LOGI(TAG, "Updated operation mode set to: automatic");
        }
        else
        {
            ret = nvs_set_i8(nvs_handle, "op_mode", 0);
            ESP_LOGI(TAG, "Updated operation mode set to: manual");
        }

        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the window degree to the storage
void nvs_write_window_deg(float window_deg)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();
    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        int64_t window_deg_t = window_deg * 100;
        ESP_LOGI(TAG, "Updating the window degree in the NVS.");
        ret = nvs_set_i64(nvs_handle, "window_deg", window_deg_t);
        ESP_LOGI(TAG, "Updated the window degree: %.2f", window_deg);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the desired temperature to the storage
void nvs_write_desired_temp(int desired_temp)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();
    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the desired temperature in the NVS.");
        ret = nvs_set_i64(nvs_handle, "desired_temp", desired_temp);
        ESP_LOGI(TAG, "Updated the desired temperature: %d", desired_temp);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the latitude to the storage
void nvs_write_latitude(float lat)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();
    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        int64_t lat_t = lat * 100;
        //Write
        ESP_LOGI(TAG, "Updating the latitude in the NVS.");
        ret = nvs_set_i64(nvs_handle, "lat", lat_t);
        ESP_LOGI(TAG, "Updated the latitude: %.2f", lat);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the longitude to the storage
void nvs_write_longitude(float lon)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();
    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        int64_t lon_t = lon * 100;
        ESP_LOGI(TAG, "Updating the longitude in the NVS.");
        ret = nvs_set_i64(nvs_handle, "lon", lon_t);
        ESP_LOGI(TAG, "Updated the longitude: %.2f", lon);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Writing the ssid to the storage
void nvs_write_timezone(const char* tz)
{
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {
        //Write
        ESP_LOGI(TAG, "Updating the timezone in the NVS");
        ret = nvs_set_str(nvs_handle, "tz", tz);
        ESP_LOGI(TAG, "Updated timezone: %s", tz);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Commit the written value..
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        ret = nvs_commit(nvs_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed");
        } 
        else
        {
            ESP_LOGI(TAG, "Done");
        }

        //Close
        nvs_close(nvs_handle);
    }
}

//Reading the ssid from the storage
const char* nvs_read_wifi_ssid()
{
    char* ssid = NULL;
    size_t size = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Reading the ssid length
        ret = nvs_get_str(nvs_handle, "ssid", NULL, &size);
        ssid = (char*)malloc(size*sizeof(char*));
        //Read
        ret = nvs_get_str(nvs_handle, "ssid", ssid, &size);
        ESP_LOGI(TAG, "Reading ssid from NVS: %s ", ssid);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }

        //Close
        nvs_close(nvs_handle);
    }
    return ssid;
}

//Reading the wifi password from the storage
const char* nvs_read_wifi_pass()
{
    char* pass = NULL;
    size_t size = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Reading the ssid length
        ret = nvs_get_str(nvs_handle, "pass", NULL, &size);
        pass = (char*)malloc(size*sizeof(char*));
        //Read
        ret = nvs_get_str(nvs_handle, "pass", pass, &size);
        ESP_LOGI(TAG, "Reading password from NVS: ***");
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }

        //Close
        nvs_close(nvs_handle);
    }
    return pass;
}

//Reading the Openweathermap API-key from the storage
const char* nvs_read_apikey()
{
    char* apikey = NULL;
    size_t size = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Reading the ssid length
        ret = nvs_get_str(nvs_handle, "apikey", NULL, &size);
        apikey = (char*)malloc(size*sizeof(char*));
        //Read
        ret = nvs_get_str(nvs_handle, "apikey", apikey, &size);
        ESP_LOGI(TAG, "Reading Openweathermap API-key from NVS: %s ", apikey);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }

        //Close
        nvs_close(nvs_handle);
    }
    return apikey;
}

//Reading the operation mode from the storage
bool nvs_read_operation_mode()
{
    int8_t op_mode = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Read
        ret = nvs_get_i8(nvs_handle, "op_mode", &op_mode);
        if(op_mode)
        {
            ESP_LOGI(TAG, "Reading operation mode from NVS: automatic");
        }
        else
        {
            ESP_LOGI(TAG, "Reading operation mode from NVS: manual");
        }
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }
        //Close
        nvs_close(nvs_handle);
    }
    if(op_mode)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

//Reading the window degree from the storage
float nvs_read_window_deg()
{
    int64_t window_deg_t = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Read
        ret = nvs_get_i64(nvs_handle, "window_deg", &window_deg_t);
        printf("Window degree = %" PRIu64 "\n", window_deg_t);
        //ESP_LOGI(TAG, "Reading window degree from NVS: %.2f ", window_deg_t);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }
        //Close
        nvs_close(nvs_handle);
    }
    return window_deg_t;
}

//Reading the desired temperature from the storage
int nvs_read_desired_temp()
{
    int64_t desired_temp_t = 20;
    int desired_temp = 20;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Read
        ret = nvs_get_i64(nvs_handle, "desired_temp", &desired_temp_t);
        desired_temp = desired_temp_t;
        ESP_LOGI(TAG, "Reading the desired temperature from NVS: %d", desired_temp);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }
        //Close
        nvs_close(nvs_handle);
    }

    return desired_temp;
}

//Reading the latitude from the storage
float nvs_read_latitude()
{
    int64_t lat_t = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Read
        ret = nvs_get_i64(nvs_handle, "lat", &lat_t);
        printf("Latitude = %" PRIu64 "\n", lat_t);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }
        //Close
        nvs_close(nvs_handle);
    }
    return lat_t;
}

//Reading the longitude from the storage
float nvs_read_longitude()
{
    int64_t lon_t = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Read
        ret = nvs_get_i64(nvs_handle, "lon", &lon_t);
        printf("Window degree = %" PRIu64 "\n", lon_t);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }
        //Close
        nvs_close(nvs_handle);
    }
    return lon_t;
}

//Reading the timezone from the storage
const char* nvs_read_timezone()
{
    char* tz = NULL;
    size_t size = 0;
    //Initializing the non-volatile storage(NVS), and checks if the NVS partitions have been corrupted.
    esp_err_t ret = init_nvs();

    //Open the NVS
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }
    else
    {   
        //Reading the ssid length
        ret = nvs_get_str(nvs_handle, "tz", NULL, &size);
        tz = (char*)malloc(size*sizeof(char*));
        //Read
        ret = nvs_get_str(nvs_handle, "tz", tz, &size);
        ESP_LOGI(TAG, "Reading timezone from NVS: %s ", tz);
        switch (ret)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "Done!");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(ret));
        }

        //Close
        nvs_close(nvs_handle);
    }
    return tz;
}