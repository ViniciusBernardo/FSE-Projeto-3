#include <string.h>

#include "request.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

#define TAG "HTTP"
#define IPSTACK_KEY CONFIG_IPSTACK_KEY
#define OPEN_WEATHER_KEY CONFIG_OPENWEATHERMAP

char response_ipstack[700];
char response_openweather[800];
int lenght_response_weather = 0;

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')//api.ipstack.com/177.64.249.226?access_key=4d29dde5797395db55f887f49b07ed61
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

            if (!esp_http_client_is_chunked_response(evt->client))
            {
                strcat(response_openweather, (char *)evt->data);
                lenght_response_weather += evt->data_len;
                response_openweather[lenght_response_weather] = '\0';
            } else
            {
                strcat(response_ipstack, (char *)evt->data);
            }
 
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
        url_encode(region_code->valuestring),
        url_encode(country_code->valuestring),
        OPEN_WEATHER_KEY
    );
    return url_openweather_map;
}

void get_weather_data()
{
    char * url_ipstack = get_ipstack_url("177.64.249.226");
    http_request(url_ipstack);

    cJSON *json_ipstack = cJSON_Parse(response_ipstack);

    char * url_openweather_map = get_openweathermap_url(json_ipstack);

    memset(response_ipstack, 0, 700);

    http_request(url_openweather_map);
    cJSON *json_openweather = cJSON_Parse(response_openweather);

    char * string = cJSON_Print(json_openweather);
    printf("%s\n", string);
}