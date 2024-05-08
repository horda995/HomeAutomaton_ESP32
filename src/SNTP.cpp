/*This module is responsible for getting the current time through the SNTP protocol.
 *Source: https://github.com/espressif/esp-idf/blob/master/examples/protocols/sntp/main/sntp_example_main.c
 */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sntp.h"
#include "credentials.h"

static const char *TAG = "SNTP";
static const char *timezone_settings = TIMEZONE_DEFAULT; 
static void obtain_time(void);
static void initialize_sntp(void);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

//TODO: Add the select timezone feature.
void configure_time(void)
{
    char strftime_buf[64];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // This block checks if time is set. If not, it will connect to the internet, and get the current time.
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to the internet and getting time over NTP.");
        obtain_time();
        // Update 'now' variable with current time
        time(&now);
    }

    //Setting the timezone and daylight savings date.
    setenv("TZ", timezone_settings, 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y.%m.%d. %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "%s", strftime_buf);
}

static void obtain_time(void)
{
    initialize_sntp(); //Wait for time to be set.
    time_t now = 0;
    struct tm timeinfo = {};
    int retry = 0;
    const int retry_count = 10;
    /*If the device is not able to get the current time after the specified amount of tries,
      it will stop trying to obtain the current time.*/
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}