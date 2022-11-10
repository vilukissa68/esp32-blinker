#include "http.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_err.h"

#define HTTP_MAX_OUTPUT_BUFFER_SIZE 1024
#define HTTP_MAX_RESP_SIZE 200
#define HTTP_DEFAULT_URL "http://worldtimeapi.org/api/timezone/Europe/Helsinki.txt"

static esp_http_client_handle_t http_client;

static const char *TAG = "http";



static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            break;
        case HTTP_EVENT_ON_DATA: {
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d, data=%s",
                     evt->data_len,
                     (char *)evt->data);
        } break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        default:
            break;
    }
    return ESP_OK;
}

void http_init() {
    esp_http_client_config_t http_config = {
        .url = HTTP_DEFAULT_URL,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = http_event_handler,
    };

        http_client = esp_http_client_init(&http_config);
        esp_http_client_perform(http_client);

        // cleanup
        esp_http_client_cleanup(http_client);
}
