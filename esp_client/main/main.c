#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "mqtt.h"
#include "wifi.h"
#include "led.h"
#include "climate.h"
#include "control.h"
#include "button.h"
#include "storage.h"

xSemaphoreHandle wifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;
xSemaphoreHandle transmitDataSemaphore;

Weather_t weather_data;
int storage_status;
char room[15];

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    storage_status = get_initialization_status(room);
    if (storage_status) {
        ESP_LOGI("Storage", "Room: %s", room);
    } else {
        ESP_LOGI("Storage", "No room stored");
    }

    wifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    transmitDataSemaphore = xSemaphoreCreateBinary();

    init_led();
    wifi_start();
    xTaskCreate(&setupMqtt, "Conecta MQTT", 4096, NULL, 1, NULL);
    init_sensor();
    xTaskCreate(&readSensors, "Leitura dht", 2048, NULL, 1, NULL);
    setupButtonHandler();
    init_button();
}
