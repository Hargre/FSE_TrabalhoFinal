
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "control.h"
#include "led.h"
#include "http_client.h"

extern xSemaphoreHandle blinkSemaphore;
extern xSemaphoreHandle wifiSemaphore;

void TaskHTTPRequestChain(void *params) {
    while (true) {
        if (xSemaphoreTake(wifiSemaphore, portMAX_DELAY)) {
            char url[128];
            ESP_LOGI("HTTP Request Task", "Request Location");
            blink_led();
            build_ipstack_url(url);
            http_request(url);
            ESP_LOGI("HTTP Request Task", "Request Weather");
            blink_led();
            build_owm_url(url);
            http_request(url);

            xSemaphoreGive(wifiSemaphore);
            vTaskDelay(REQUEST_TIMER / portTICK_PERIOD_MS);
        }
    }
}

void TaskBlink(void *params) {
    while (true) {
        if (xSemaphoreTake(blinkSemaphore, portMAX_DELAY)) {
            blink_led();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            xSemaphoreGive(blinkSemaphore);
        }
    }
}