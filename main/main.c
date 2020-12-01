#include <stdio.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "request.h"
#include "led.h"

#define LED 2

xSemaphoreHandle conexaoWifiSemaphore;
TaskHandle_t ledTaskHandler = NULL;

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

void led_task(void * params)
{
  struct led_state_machine * state_machine = initialize_led();
  while (true)
  {
    int state = ulTaskNotifyTake(pdTRUE, 0);
    control_led(state_machine, state);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{
    xTaskCreate(&led_task, "Led Task", 2048, NULL, 2, &ledTaskHandler);
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskNotify(ledTaskHandler, 2, eSetValueWithOverwrite);
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&RealizaHTTPRequest,  "Processa HTTP", 4096, NULL, 2, NULL);
}