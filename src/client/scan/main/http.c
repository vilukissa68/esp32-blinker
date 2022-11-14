#include "http.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"
#include "freertos/ringbuf.h"

#define HTTP_MAX_OUTPUT_BUFFER_SIZE 1024
#define HTTP_MAX_RESP_SIZE 200
#define HTTP_DEFAULT_URL "http://worldtimeapi.org/api/timezone/Europe/Helsinki.json"

static esp_http_client_handle_t http_client;

static const char *TAG = "http";

RingbufHandle_t xRingbuffer;

static void http_json_parser(const cJSON * const root) {
    ESP_LOGI(TAG, "parsing json");
    cJSON *day = cJSON_GetObjectItem(root, "day_of_week");
    ESP_LOGI(TAG, "Day of week: %s", cJSON_Print(day));
}

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
            if (!esp_http_client_is_chunked_response(evt->client)) {
                char *recv_buffer = malloc(evt->data_len + 1);
                memcpy(recv_buffer, evt->data, evt->data_len);
                recv_buffer[evt->data_len] = 0; // Confirm that buffer has ended
                UBaseType_t response = xRingbufferSendFromISR(xRingbuffer, recv_buffer, evt->data_len, NULL);
                ESP_LOGI(TAG, "xRingbufferSendFromISR res=%d", response);
                if (response != pdTRUE) {
                    ESP_LOGI(TAG, "Failed to xRingbufferSend");
                }
                free(recv_buffer);
            }
            } break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void http_ringbuffer_init() {
    xRingbuffer = xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT);
    configASSERT(xRingbuffer);

}

void http_init() {
    esp_http_client_config_t http_config = {
        .url = HTTP_DEFAULT_URL,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = http_event_handler,
    };

    http_ringbuffer_init();
    http_client = esp_http_client_init(&http_config);
    esp_err_t err = esp_http_client_perform(http_client);

    // If OK read from ring buffer
    if (err == ESP_OK) {
        int buffer_size = esp_http_client_get_content_length(http_client);
        char *buffer = malloc(buffer_size + 1);
        size_t item_size;
        int index = 0;
        while(1) {
            char *item = (char *)xRingbufferReceive(xRingbuffer, &item_size, pdMS_TO_TICKS(1000));
            if (item == NULL) {
                ESP_LOGI(TAG, "End of recive item");
                break;
            }
            for (int i = 0; i < item_size; i++) {
                buffer[index] = item[i];
                index++;
                buffer[index] = 0;
            }
            vRingbufferReturnItem(xRingbuffer, (void *)item);
        }

        //ESP_LOGI(TAG, "buffer:\n%s", buffer);
        cJSON *root = cJSON_Parse(buffer);
        http_json_parser(root);
        cJSON_Delete(root);
        free(buffer);
    }

    // cleanup
    esp_http_client_cleanup(http_client);
}
