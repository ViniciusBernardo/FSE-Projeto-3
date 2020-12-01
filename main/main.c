#include <stdio.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"


#include "wifi.h"
#include "request.h"

#define LED 2

xSemaphoreHandle conexaoWifiSemaphore;
TaskHandle_t receptorHandler = NULL;

void RealizaHTTPRequest(void * params)
{
  if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
  {
    while(true)
    {
      vTaskDelay(2e4 / portTICK_PERIOD_MS);
      ESP_LOGI("Main Task", "Realiza HTTP Request");
      get_weather_data();
    }
  }

}

void receptor(void * params)
{
  gpio_pad_select_gpio(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  int led = 1;
  int previous_state = 1;
  while (true)
  {
    // state = 0 -> keep previous state
    // state = 1 -> keep LED on
    // state = 2 -> Blink
    // state = 3 -> Blink ONCE
    int state = ulTaskNotifyTake(pdTRUE, 0);
    if((state == 1 || previous_state == 1) && (state != 2 && state != 3)){
      led = 1;
      gpio_set_level(LED, led);
      previous_state = 1;
    } else if((state == 2 || previous_state == 2) && (state != 1 && state != 3)){
      led = !led;
      gpio_set_level(LED, led);
      previous_state = 2;
    } else if(state == 3){
      gpio_set_level(LED, 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      gpio_set_level(LED, 1);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{
    xTaskCreate(&receptor, "Receptor", 2048, NULL, 2, &receptorHandler);
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskNotify(receptorHandler, 2, eSetValueWithOverwrite);
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&RealizaHTTPRequest,  "Processa HTTP", 4096, NULL, 2, NULL);
}