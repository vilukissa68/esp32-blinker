#include "esp_wifi.h"
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "config.h"

#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_MAXIMUM_RETRY 3

static const char *TAG = "scan";
static EventGroupHandle_t m_wifi_event_group;

static int m_retry_count = 0;

wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START: {
            ESP_LOGI(TAG, "Wifi connecting...");
            ESP_ERROR_CHECK(esp_wifi_connect());
        } break;
        case WIFI_EVENT_STA_CONNECTED: {
            ESP_LOGI(TAG, "Wifi connected");
        } break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            ESP_LOGI(TAG, "Wifi disconnected");
            if (m_retry_count < WIFI_MAXIMUM_RETRY) {
                esp_wifi_connect();
                m_retry_count++;
                ESP_LOGI(TAG, "retry to connect to AP");
            } else {
                xEventGroupSetBits(m_wifi_event_group, WIFI_FAIL_BIT);
            }
        } break;
        case IP_EVENT_STA_GOT_IP: {
            ESP_LOGI(TAG, "Wifi got IP address");
            xEventGroupSetBits(m_wifi_event_group, WIFI_CONNECTED_BIT);
        } break;
        default:
            break;
    }
}

void wifi_init() {

    m_wifi_event_group = xEventGroupCreate();

    // Initialize wifi
    ESP_ERROR_CHECK(esp_netif_init()); // TCP/IP initialization
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Event loop
    esp_netif_create_default_wifi_sta(); // Wifi station
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Set wifi configuration
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    //ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for wifi to connect
    EventBits_t bits = xEventGroupWaitBits(m_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID:%s", EXAMPLE_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", EXAMPLE_WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "Unexpected event during wifi connection");
    }
}
