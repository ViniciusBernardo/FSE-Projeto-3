#include <string.h>

#include "request.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

#define TAG "HTTP"
#define IPSTACK_KEY CONFIG_IPSTACK_KEY

char ipstack_message[400];

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

            strcat(ipstack_message, (char *)evt->data);
 
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void ipstack(const char * ip_adrees)
{
    char * url = malloc(100*sizeof(char));
    sprintf(
        url,
        "http://api.ipstack.com/%s?access_key=%s",
        ip_adrees,
        IPSTACK_KEY
    );
    ESP_LOGI("IPSTACK", "URL: %s", url);
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handle
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d", esp_http_client_get_status_code(client));
        printf("%ss", ipstack_message);
    }
    esp_http_client_cleanup(client);
}
