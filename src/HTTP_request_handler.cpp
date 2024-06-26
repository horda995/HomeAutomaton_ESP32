/* This module uses Mbed-TLS to connect to the openweathermap API, and get the necessary weather data from their server. Used source:
 *
 * https://github.com/espressif/esp-idf/blob/master/examples/protocols/https_mbedtls/main/https_mbedtls_example_main.c
 * 
 * License: Apache-2.0
 * 2015-2021 Espressif Systems (Shanghai) CO LTD
 */


#include <stdlib.h>
#include <inttypes.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_crt_bundle.h"
#include "JSON_parser.h"
#include "credentials.h"
#include "store_data.h"
#include "weather_data.h"

using namespace std;

#define WEB_SERVER "api.openweathermap.org"
#define WEB_PORT "443"

extern Weather_data Weather;
string openweathermap_app_id = nvs_read_apikey();
static const char *TAG = "HTTPS_REQUEST";
float latitude = LAT, longitude = LON;

string GET_REQUEST(float latitude, float longitude, string openweathermap_app_id){
    /*
     * This function constructs a request string that will be sent to the server.The string should look like this:
     * https://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&units=metric&exclude=minutely,hourly,daily&appid=%s"
     *
     */
    string WEB_URL = "/data/2.5/onecall?lat=" +
                     to_string(latitude) +
                     "&lon=" +
                     to_string(longitude) + 
                     "&units=metric&exclude=minutely,hourly,daily&appid=" +
                     openweathermap_app_id;
                     
    string REQUEST = "GET " + WEB_URL + " HTTP/1.0\r\n" +
    "Host: " + WEB_SERVER + "\r\n" 
    "User-Agent: esp-idf/1.0 esp32\r\n" +
    "Accept: application/json\r\n" +
    "Content-Type: application/json\r\n" + 
    "\r\n";;
    return REQUEST;
}

void https_request_task(void *pvParameters)
{
    char buf[512];
    int ret, flags, len;
    size_t written_bytes;
    string api_response;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;
    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    string REQUEST = GET_REQUEST(Weather.get_lat(), Weather.get_lon(), openweathermap_app_id);
    
    mbedtls_ssl_config_init(&conf); //Initializing mbedtls.
    mbedtls_entropy_init(&entropy);
    ESP_LOGI(TAG, "Seeding the random number generator");
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    ESP_LOGI(TAG, "Attaching the certificate bundle...");

    ret = esp_crt_bundle_attach(&conf);

    if (ret < 0)
    {
        ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");

    if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    while(1) 
    {
        //Using Mbed-TLS to connect to the server, and set up SSL/TLS communication
        mbedtls_net_init(&server_fd);
        ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        ESP_LOGI(TAG, "Connected.");
        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
        ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            ESP_LOGW(TAG, "Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            ESP_LOGW(TAG, "verification info: %s", buf);
            goto exit;
        }
        else
        {
            ESP_LOGI(TAG, "Certificate verified.");
        }

        ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

        ESP_LOGI(TAG, "Writing HTTP request...");

        written_bytes = 0;
        do {
            ret = mbedtls_ssl_write(&ssl,
                                    (const unsigned char *)REQUEST.c_str() + written_bytes,
                                    strlen(REQUEST.c_str()) - written_bytes);
            if (ret >= 0) 
            {
                ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
            } 
            else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
                goto exit;
            }
        } while(written_bytes < strlen(REQUEST.c_str()));

        ESP_LOGI(TAG, "Reading HTTP response...");
        
        do {
            len = sizeof(buf) - 1;
            bzero(buf, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);
            
            if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                continue;

            if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
            }

            if(ret < 0)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
                break;
            }

            if(ret == 0)
            {
                ESP_LOGI(TAG, "connection closed");
                break;
            }

            len = ret;
            ESP_LOGD(TAG, "%d bytes read", len);
            for(int i = 0; i < len; i++) {
                if(api_response != "")
                    api_response += buf[i];
                else
                    api_response = buf[i];
            }
            
        } while(1);
        //ESP_LOGI(TAG, "%s\n", api_response.c_str()); 
        mbedtls_ssl_close_notify(&ssl);
        parse_weather_json(api_response);
        api_response = "";

    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if (ret != 0)
        {
            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        }

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);
        vTaskDelay(600000 / portTICK_PERIOD_MS);
    }
}

