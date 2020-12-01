#include <string.h>

#include "request.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

#define TAG "HTTP"
#define IPSTACK_KEY CONFIG_IPSTACK_KEY
#define OPEN_WEATHER_KEY CONFIG_OPENWEATHERMAP

char response_string[800];
int lenght_response = 0;
extern TaskHandle_t receptorHandler;

char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

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

            strcat(response_string, (char *)evt->data);
            lenght_response += evt->data_len;
            response_string[lenght_response] = '\0';

 
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

void http_request(char * url)
{
    ESP_LOGI("HTTP REQUEST", "URL: %s", url);
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handle
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d", esp_http_client_get_status_code(client));
    }
    esp_http_client_cleanup(client);
}

char * get_ipstack_url(char * ip_address)
{
    char * url_ipstack = malloc(100*sizeof(char));
    sprintf(
        url_ipstack,
        "http://api.ipstack.com/%s?access_key=%s",
        ip_address,
        IPSTACK_KEY
    );
    return url_ipstack;
}

void print_json(cJSON * json)
{
    char *string = cJSON_Print(json);
    printf("%s\n", string);
}

char * get_openweathermap_url(cJSON * json)
{
    const cJSON *country_code = NULL;
    const cJSON *region_code = NULL;
    const cJSON *city = NULL;

    country_code = cJSON_GetObjectItemCaseSensitive(json, "country_code");
    region_code = cJSON_GetObjectItemCaseSensitive(json, "region_code");
    city = cJSON_GetObjectItemCaseSensitive(json, "city");

    char * url_openweather_map = (char *)malloc(150*sizeof(char));
    sprintf(
        url_openweather_map,
        "http://api.openweathermap.org/data/2.5/weather?q=%s,%s,%s&units=metric&appid=%s",
        url_encode(city->valuestring),
        region_code->valuestring,
        country_code->valuestring,
        OPEN_WEATHER_KEY
    );
    return url_openweather_map;
}

void show_weather_data(cJSON * json_openweather){
    cJSON *main_info = cJSON_GetObjectItemCaseSensitive(json_openweather, "main");
    cJSON *current_temperature = cJSON_DetachItemFromObjectCaseSensitive(main_info, "temp");
    cJSON *minimum_temperature = cJSON_DetachItemFromObjectCaseSensitive(main_info, "temp_min");
    cJSON *maximum_temperature = cJSON_DetachItemFromObjectCaseSensitive(main_info, "temp_max");
    cJSON *humidity = cJSON_DetachItemFromObjectCaseSensitive(main_info, "humidity");

    printf("Temperatura Máxima: %.2lf | Temperatura Mínima: %.2lf | Temperatura Atual: %.2lf | Umidade: %.2lf %%\n",
           maximum_temperature->valuedouble,
           current_temperature->valuedouble,
           minimum_temperature->valuedouble,
           humidity->valuedouble);
}

void get_weather_data()
{
    char * url_ipstack = get_ipstack_url("177.64.249.226");

    response_string[0] = '\0';
    lenght_response = 0;

    xTaskNotify(receptorHandler, 3, eSetValueWithOverwrite);
    http_request(url_ipstack);

    cJSON *json_ipstack = cJSON_Parse(response_string);
    char * url_openweather_map = get_openweathermap_url(json_ipstack);

    response_string[0] = '\0';
    lenght_response = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskNotify(receptorHandler, 3, eSetValueWithOverwrite);
    http_request(url_openweather_map);

    cJSON *json_openweather = cJSON_Parse(response_string);

    show_weather_data(json_openweather);
}