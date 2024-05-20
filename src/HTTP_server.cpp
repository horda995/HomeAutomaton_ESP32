/* This module is the implementation of a simple HTTP webserver, that enables the change of
 * the Wi-Fi credentials and the Openweathermap API-key, also it makes some minor configurations
 * possible regarding the window opener.
 */

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_https_server.h>
#include <sys/stat.h>
#include "esp_spiffs.h"
#include "room_data.h"
#include "weather_data.h"
#include "WiFi_STA.h"
#include <string>
#include "HTTP_request_handler.h"
#include "store_data.h"

static const char *TAG = "HTTPS_SERVER";
#define CONFIG_PAGE_HTML_PATH "/spiffs/config_page.html"
char config_html[16384];
char response_data[16384];
extern Room_data Internal_room_data;
extern Weather_data Weather;
extern string openweathermap_app_id;
extern TaskHandle_t http_request_task_handle;

//Because the data is URL encoded, it is necessary to decode it.
string data_decode(string data)
{
    string decoded_data = "";
    for (size_t i = 0; i < data.size(); i++) {
        if (data[i] == '%' && i + 2 < data.size() && isxdigit(data[i + 1]) && isxdigit(data[i + 2])) {
            //If it's a percent-encoded
            int value;
            sscanf(data.substr(i + 1, 2).c_str(), "%x", &value);
            decoded_data += static_cast<char>(value);
            i += 2; // Move ahead by 2 characters
        } else if (data[i] == '+') {
            //Replace '+' with space
            decoded_data += ' ';
        } else {
            //Copy the character
            decoded_data += data[i];
        }
    }
    
    return decoded_data;
}

//Parsing the data from the url
string parse_url(string url_response, string data)
{
    //Find the data in the url
    data = data + "=";
    size_t position = url_response.find(data);
    //Execute the code only if the data has been found
    if (position != string::npos)
    {
        //Finds the position where our data starts and where it ends
        size_t start = position + data.length();
        size_t end = url_response.find_first_of("&", start);
        //Copies the substring where the data is located.
        if (end == string::npos)
        {
            data = url_response.substr(start);
        }
        else
        {
            data = url_response.substr(start, end-start);
        }
        return data;
    }
    else
    {
        return "";
    }
}

void config_web_page_buffer()
{
    //Define the flash SPIFFS partition
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    //Read the file from the filesystem
    memset((void *)config_html, 0, sizeof(config_html));
    struct stat st;
    if (stat(CONFIG_PAGE_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "config_page.html not found");
        return;
    }

    FILE *fp = fopen(CONFIG_PAGE_HTML_PATH, "r");
    if (fread(config_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}

void fill_config_page()
{
    /*Depending on the operation mode of the window opener (auto or manual)
      The radio-buttons on the config page must be checked accordingly.
      This part of the code writes in the config page.
     */
    
    if (Internal_room_data.get_is_auto())
    {
        sprintf(response_data, config_html, Internal_room_data.get_window_deg()*100,
                                            Internal_room_data.get_desired_temperature(),
                                            Internal_room_data.get_desired_temperature(),
                                            "true");
    }
    else
    {
        sprintf(response_data, config_html, Internal_room_data.get_window_deg()*100,
                                            Internal_room_data.get_desired_temperature(),
                                            Internal_room_data.get_desired_temperature(),
                                            "false");
    }
    /*
    if (Internal_room_data.get_is_auto())
    {
        sprintf(response_data, config_html, "true");
    }
    else
    {
        sprintf(response_data, config_html, "false");
    }*/
}

//The HTTP-GET handler of the config page which returns the page itself.
static esp_err_t root_get_handler(httpd_req_t *req)
{
    fill_config_page();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t root_get = 
{
    .uri = "/",
    .method  = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx  = NULL,
    .is_websocket = NULL,
    .handle_ws_control_frames = NULL,
    .supported_subprotocol = NULL
};

//Handling the HTTP-POST requests of the client.
static esp_err_t submit_wifi_post_handler(httpd_req_t *req)
{
    
    ESP_LOGI(TAG, "/ content length %d", req->content_len);

    char*  buf = (char*)malloc(req->content_len + 1);
    size_t off = 0;
    int ret;
    string ssid, pass;

    if (!buf)
    {
        ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", req->content_len + 1);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (off < req->content_len)
    {
        //Read data received in the request
        ret = httpd_req_recv(req, buf + off, req->content_len - off);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            free (buf);
            return ESP_FAIL;
        }
        off += ret;
        ESP_LOGI(TAG, "/ Recv length %d", ret);
    }
    buf[off] = '\0';

    if (req->content_len < 128) {
        ESP_LOGI(TAG, "/ data: %s", buf);
    }

    //Search for Custom header field
    char*  req_hdr = 0;
    size_t hdr_len = httpd_req_get_hdr_value_len(req, "Custom");
    if (hdr_len)
    {
        //Read Custom header value
        req_hdr = (char*)malloc(hdr_len + 1);
        if (!req_hdr)
        {
            ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", hdr_len + 1);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        httpd_req_get_hdr_value_str(req, "Custom", req_hdr, hdr_len + 1);

        //Set as additional header for response packet
        httpd_resp_set_hdr(req, "Custom", req_hdr);
    }
 
    ssid = parse_url(buf, "ssid");
    ssid = data_decode(ssid);
    ESP_LOGI(TAG, "The SSID has been updated: %s", ssid.c_str());

    pass = parse_url(buf, "password");
    pass = data_decode(pass);
    ESP_LOGI(TAG, "The password has been updated: ***");

    nvs_write_wifi_ssid(ssid.c_str());
    nvs_write_wifi_pass(pass.c_str());

    fill_config_page();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
    //End response
    free (req_hdr);
    free (buf);

    ssid = nvs_read_wifi_ssid();
    pass = nvs_read_wifi_pass();
    wifi_restart(ssid.c_str(), pass.c_str());

    return ESP_OK;
}

static esp_err_t submit_api_key_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "/ content length %d", req->content_len);

    char*  buf = (char*)malloc(req->content_len + 1);
    size_t off = 0;
    int ret;

    if (!buf)
    {
        ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", req->content_len + 1);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (off < req->content_len)
    {
        //Read data received in the request
        ret = httpd_req_recv(req, buf + off, req->content_len - off);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            free (buf);
            return ESP_FAIL;
        }
        off += ret;
        ESP_LOGI(TAG, "/ Recv length %d", ret);
    }
    buf[off] = '\0';

    if (req->content_len < 128) {
        ESP_LOGI(TAG, "/ data: %s", buf);
    }

    //Search for Custom header field
    char*  req_hdr = 0;
    size_t hdr_len = httpd_req_get_hdr_value_len(req, "Custom");
    if (hdr_len)
    {
        //Read Custom header value
        req_hdr = (char*)malloc(hdr_len + 1);
        if (!req_hdr)
        {
            ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", hdr_len + 1);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        httpd_req_get_hdr_value_str(req, "Custom", req_hdr, hdr_len + 1);

        //Set as additional header for response packet
        httpd_resp_set_hdr(req, "Custom", req_hdr);
    }
    
    //Parsing the Openweathermap API-key and restarting the request task that gets the data from the API
    if (parse_url(buf, "apikey") != "")
    {
        openweathermap_app_id = parse_url(buf, "apikey");
        openweathermap_app_id = data_decode(openweathermap_app_id);
        nvs_write_apikey(openweathermap_app_id.c_str());
        vTaskDelete(http_request_task_handle);
        xTaskCreate(&https_request_task, "https_request_task", 8192, NULL, 5, &http_request_task_handle);
        ESP_LOGI(TAG, "The Openweathermap API-key has been updated!");
    }
    
    fill_config_page();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
    //End response
    free (req_hdr);
    free (buf);

    return ESP_OK;
}

static esp_err_t submit_mode_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "/ content length %d", req->content_len);

    char*  buf = (char*)malloc(req->content_len + 1);
    size_t off = 0;
    int ret;

    if (!buf)
    {
        ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", req->content_len + 1);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (off < req->content_len)
    {
        //Read data received in the request
        ret = httpd_req_recv(req, buf + off, req->content_len - off);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            free (buf);
            return ESP_FAIL;
        }
        off += ret;
        ESP_LOGI(TAG, "/ Recv length %d", ret);
    }
    buf[off] = '\0';

    if (req->content_len < 128) {
        ESP_LOGI(TAG, "/ data: %s", buf);
    }

    //Search for Custom header field
    char*  req_hdr = 0;
    size_t hdr_len = httpd_req_get_hdr_value_len(req, "Custom");
    if (hdr_len)
    {
        //Read Custom header value
        req_hdr = (char*)malloc(hdr_len + 1);
        if (!req_hdr)
        {
            ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", hdr_len + 1);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        httpd_req_get_hdr_value_str(req, "Custom", req_hdr, hdr_len + 1);

        //Set as additional header for response packet
        httpd_resp_set_hdr(req, "Custom", req_hdr);
    }
    
    //Parsing the data regarding the operation mode of the microcontroller
    if (parse_url(buf, "choosemode") == "auto")
    {
        Internal_room_data.set_is_auto(true);
        ESP_LOGI(TAG, "The operation mode has been set to automatic!");
        nvs_write_operation_mode(Internal_room_data.get_is_auto());
    }
    else if (parse_url(buf, "choosemode") == "manual")
    {
        Internal_room_data.set_is_auto(false);
        ESP_LOGI(TAG, "The operation mode has been set to manual!");
        nvs_write_operation_mode(Internal_room_data.get_is_auto());
    }
    if (parse_url(buf, "temp") != "")
    {
        Internal_room_data.set_desired_temperature(stoi(parse_url(buf, "temp")));
        ESP_LOGI(TAG, "Desired temperature has been changed to: %d", Internal_room_data.get_desired_temperature());
        nvs_write_desired_temp(Internal_room_data.get_desired_temperature());
    }
    if (parse_url(buf, "window") != "")
    {
        Internal_room_data.set_window_deg(stof(parse_url(buf, "window"))/100);
        ESP_LOGI(TAG, "Window angle has been changed to : %f", Internal_room_data.get_window_deg());
        nvs_write_window_deg(Internal_room_data.get_window_deg());
    }
    
    fill_config_page();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
    //End response
    free (req_hdr);
    free (buf);

    return ESP_OK;
}

static const httpd_uri_t submit_wifi_post =
{
    .uri = "/SubmitWiFi",
    .method = HTTP_POST,
    .handler  = submit_wifi_post_handler,
    .user_ctx  = NULL,
    .is_websocket = NULL,
    .handle_ws_control_frames = NULL,
    .supported_subprotocol = NULL
};

static const httpd_uri_t submit_api_key_post =
{
    .uri = "/SubmitAPI",
    .method = HTTP_POST,
    .handler  = submit_api_key_post_handler,
    .user_ctx  = NULL,
    .is_websocket = NULL,
    .handle_ws_control_frames = NULL,
    .supported_subprotocol = NULL
};

static const httpd_uri_t submit_mode_post =
{
    .uri = "/SubmitMode",
    .method = HTTP_POST,
    .handler  = submit_mode_post_handler,
    .user_ctx  = NULL,
    .is_websocket = NULL,
    .handle_ws_control_frames = NULL,
    .supported_subprotocol = NULL
};


httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    //Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        //Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root_get);
        httpd_register_uri_handler(server, &submit_wifi_post);
        httpd_register_uri_handler(server, &submit_api_key_post);
        httpd_register_uri_handler(server, &submit_mode_post);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    //Stop the httpd server
    return httpd_ssl_stop(server);
}

void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server)
    {
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop https server");
        }
    }
}

void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}
